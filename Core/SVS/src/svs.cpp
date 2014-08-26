#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <utility>
#include <algorithm>
#include <fstream>

#include "svs.h"
#include "command.h"
#include "sgnode.h"
#include "soar_interface.h"
#include "scene.h"
#include "common.h"
#include "filter_table.h"
#include "command_table.h"
#include "drawer.h"

#include "symtab.h"

using namespace std;

typedef map<string, command*>::iterator cmd_iter;

svs_interface* make_svs(agent* a)
{
    return new svs(a);
}


sgwme::sgwme(soar_interface* si, Symbol* ident, sgwme* parent, sgnode* node)
    : soarint(si), id(ident), parent(parent), node(node)
{
    node->listen(this);
    name_wme = soarint->make_wme(id, si->get_common_syms().id, node->get_name());
    
    if (node->is_group())
    {
        group_node* g = node->as_group();
        for (int i = 0; i < g->num_children(); ++i)
        {
            add_child(g->get_child(i));
        }
    }

    const tag_map& node_tags = node->get_all_tags();
    tag_map::const_iterator ti;
    for(ti = node_tags.begin(); ti != node_tags.end(); ti++){
      set_tag(ti->first, ti->second);
    }
}

sgwme::~sgwme()
{
    map<sgwme*, wme*>::iterator i;
    
    if (node)
    {
        node->unlisten(this);
    }
    soarint->remove_wme(name_wme);

    map<string, wme*>::iterator ti;
    for(ti = tags.begin(); ti != tags.end(); ti++){
      soarint->remove_wme(ti->second);
    }
    
    for (i = childs.begin(); i != childs.end(); ++i)
    {
        i->first->parent = NULL;
        delete i->first;
        soarint->remove_wme(i->second);
    }
    if (parent)
    {
        map<sgwme*, wme*>::iterator ci = parent->childs.find(this);
        assert(ci != parent->childs.end());
        soarint->remove_wme(ci->second);
        parent->childs.erase(ci);
    }
}

void sgwme::node_update(sgnode* n, sgnode::change_type t, const std::string& update_info)
{
    int added_child = 0;
    group_node* g;
    switch (t)
    {
        case sgnode::CHILD_ADDED:
            if (parse_int(update_info, added_child))
            {
                g = node->as_group();
                add_child(g->get_child(added_child));
            }
            break;
        case sgnode::DELETED:
            node = NULL;
            delete this;
            break;
        case sgnode::TAG_CHANGED:
            update_tag(update_info);
            break;
        case sgnode::TAG_DELETED:
            delete_tag(update_info);
            break;
        default:
            break;
    };
}

void sgwme::add_child(sgnode* c)
{
    char letter;
    string cname = c->get_name();
    sgwme* child;
    
    if (cname.size() == 0 || !isalpha(cname[0]))
    {
        letter = 'n';
    }
    else
    {
        letter = cname[0];
    }
    wme* cid_wme = soarint->make_id_wme(id, "child");
    
    child = new sgwme(soarint, soarint->get_wme_val(cid_wme), this, c);
    childs[child] = cid_wme;
}

void sgwme::set_tag(const string& tag_name, const string& tag_value)
{
    Symbol* rootID = id;
    std::string att = tag_name;

    wme* value_wme;
    if(map_get(tags, tag_name, value_wme)){
      soarint->remove_wme(value_wme);
    }
    tags[tag_name] = soarint->make_wme(rootID, att, tag_value);
}

void sgwme::update_tag(const string& tag_name)
{
    string tag_value;
    if(node->get_tag(tag_name, tag_value)){
      set_tag(tag_name, tag_value);
    }
}

void sgwme::delete_tag(const string& tag_name)
{
  wme* value_wme;
  if(map_get(tags, tag_name, value_wme)){
    soarint->remove_wme(value_wme);
    tags.erase(tag_name);
  }
}


svs_state::svs_state(svs* svsp, Symbol* state, soar_interface* si, scene* scn)
    : svsp(svsp), parent(NULL), state(state), si(si), level(0),
      scene_num(-1), scene_num_wme(NULL), scn(scn), scene_link(NULL)
{
    assert(state->is_top_state());
    state->get_id_name(name);
    init();
}

svs_state::svs_state(Symbol* state, svs_state* parent)
    : parent(parent), state(state), svsp(parent->svsp), si(parent->si),
      level(parent->level + 1), scene_num(-1),
      scene_num_wme(NULL), scn(NULL), scene_link(NULL)
{
    assert(state->get_parent_state() == parent->state);
    init();
}

svs_state::~svs_state()
{
    command_set_it i, iend;
    
    for (i = curr_cmds.begin(), iend = curr_cmds.end(); i != iend; ++i)
    {
        delete i->cmd;
    }
    
    if (scn)
    {
        svsp->get_drawer()->delete_scene(scn->get_name());
        delete scn; // results in root being deleted also
    }
}

void svs_state::init()
{
    common_syms& cs = si->get_common_syms();
    
    state->get_id_name(name);
    svs_link = si->get_wme_val(si->make_id_wme(state, cs.svs));
    cmd_link = si->get_wme_val(si->make_id_wme(svs_link, cs.cmd));
    scene_link = si->get_wme_val(si->make_id_wme(svs_link, cs.scene));
    if (!scn)
    {
        if (parent)
        {
            scn = parent->scn->clone(name);
        }
        else
        {
            // top state
            scn = new scene(name, svsp);
            scn->set_draw(true);
        }
    }
    scn->refresh_draw();
    root = new sgwme(si, scene_link, (sgwme*) NULL, scn->get_root());
}

void svs_state::update_scene_num()
{
    long curr;
    if (scene_num_wme)
    {
        if (!get_symbol_value(si->get_wme_val(scene_num_wme), curr))
        {
            exit(1);
        }
        if (curr == scene_num)
        {
            return;
        }
        si->remove_wme(scene_num_wme);
    }
    if (scene_num >= 0)
    {
        scene_num_wme = si->make_wme(svs_link, "scene-num", scene_num);
    }
}

void svs_state::update_cmd_results(bool early)
{
    command_set_it i;
    for (i = curr_cmds.begin(); i != curr_cmds.end(); ++i)
    {
        if (i->cmd->early() == early)
        {
            i->cmd->update();
        }
    }
}

//#include <iostream>
//using namespace std;

void svs_state::process_cmds()
{
    wme_list all;
    wme_list::iterator all_it;
    si->get_child_wmes(cmd_link, all);
    
    command_set live_commands;
    for (all_it = all.begin(); all_it != all.end(); all_it++)
    {
        // Convert wme val to string
        Symbol* idSym = si->get_wme_val(*all_it);
        string cmdId;
        ;
        if (!idSym->get_id_name(cmdId))
        {
            // Not an identifier, continue;
            continue;
        }
        
        live_commands.insert(command_entry(cmdId, 0, *all_it));
    }
    // Do a diff on the curr_cmds list and the live_commands
    //   to find which have been added and which have been removed
    vector<command_set_it> old_commands, new_commands;
    command_set_it live_it = live_commands.begin();
    command_set_it curr_it = curr_cmds.begin();
    while (live_it != live_commands.end() || curr_it != curr_cmds.end())
    {
        if (live_it == live_commands.end())
        {
            old_commands.push_back(curr_it);
            curr_it++;
        }
        else if (curr_it == curr_cmds.end())
        {
            new_commands.push_back(live_it);
            live_it++;
        }
        else if (curr_it->id == live_it->id)
            // Find all commands removed from the svs command wme
        {
            curr_it++;
            live_it++;
        }
        else if (curr_it->id.compare(live_it->id) < 0)
        {
            old_commands.push_back(curr_it);
            curr_it++;
        }
        else
        {
            new_commands.push_back(live_it);
            live_it++;
        }
    }
    
    // Delete the command
    vector<command_set_it>::iterator old_it;
    for (old_it = old_commands.begin(); old_it != old_commands.end(); old_it++)
    {
        command_set_it old_cmd = *old_it;
        delete old_cmd->cmd;
        curr_cmds.erase(old_cmd);
    }
    
    // Add the new commands
    vector<command_set_it>::iterator new_it;
    for (new_it = new_commands.begin(); new_it != new_commands.end(); new_it++)
    {
        command_set_it new_cmd = *new_it;
        command* c = get_command_table().make_command(this, new_cmd->cmd_wme);
        if (c)
        {
            curr_cmds.insert(command_entry(new_cmd->id, c, 0));
        }
        else
        {
            string attr;
            get_symbol_value(si->get_wme_attr(new_cmd->cmd_wme), attr);
        }
    }
}

void svs_state::clear_scene()
{
    scn->clear();
}

void svs_state::proxy_get_children(map<string, cliproxy*>& c)
{
    c["scene"]        = scn;
}

void svs_state::disown_scene()
{
    delete root;
    scn = NULL;
}

svs::svs(agent* a)
    : scn_cache(NULL)
{
    si = new soar_interface(a);
    draw = new drawer();
}

svs::~svs()
{
    for (int i = 0, iend = state_stack.size(); i < iend; ++i)
    {
        delete state_stack[i];
    }
    if (scn_cache)
    {
        delete scn_cache;
    }
    
    delete si;
    delete draw;
}

void svs::state_creation_callback(Symbol* state)
{
    string type, msg;
    svs_state* s;
    
    if (state_stack.empty())
    {
        if (scn_cache)
        {
            scn_cache->verify_listeners();
        }
        s = new svs_state(this, state, si, scn_cache);
        scn_cache = NULL;
    }
    else
    {
        s = new svs_state(state, state_stack.back());
    }
    
    state_stack.push_back(s);
}

void svs::state_deletion_callback(Symbol* state)
{
    svs_state* s;
    s = state_stack.back();
    assert(state == s->get_state());
    if (state_stack.size() == 1)
    {
        // removing top state, save scene for reinit
        scn_cache = s->get_scene();
        s->disown_scene();
    }
    delete s;
    state_stack.pop_back();
}

void svs::proc_input(svs_state* s)
{
    for (int i = 0; i < env_inputs.size(); ++i)
    {
        strip(env_inputs[i], " \t");
        s->get_scene()->parse_sgel(env_inputs[i]);
    }
    env_inputs.clear();
}

void svs::output_callback()
{
    vector<svs_state*>::iterator i;
    string sgel;
    svs_state* topstate = state_stack.front();
    
    for (i = state_stack.begin(); i != state_stack.end(); ++i)
    {
        (**i).process_cmds();
    }
    for (i = state_stack.begin(); i != state_stack.end(); ++i)
    {
        (**i).update_cmd_results(true);
    }
    
}

void svs::input_callback(){
    svs_state* topstate = state_stack.front();
    proc_input(topstate);
    
    vector<svs_state*>::iterator i;
    for (i = state_stack.begin(); i != state_stack.end(); ++i)
    {
        (**i).update_cmd_results(false);
    }
}

/*
 This is a naive implementation. If this method is called concurrently
 with proc_input, the env_inputs vector will probably become
 inconsistent. This eventually needs to be replaced by a thread-safe FIFO.
*/
void svs::add_input(const string& in)
{
    split(in, "\n", env_inputs);
}

string svs::svs_query(const string& query)
{
    if (state_stack.size() == 0)
    {
        return "";
    }
    return state_stack[0]->get_scene()->parse_query(query);
}

void svs::proxy_get_children(map<string, cliproxy*>& c)
{
    c["connect_viewer"]    = new memfunc_proxy<svs>(this, &svs::cli_connect_viewer);
    c["connect_viewer"]->set_help("Connect to a running viewer.")
    .add_arg("PORT", "TCP port (or file socket path in Linux) to connect to.")
    ;
    
    c["disconnect_viewer"] = new memfunc_proxy<svs>(this, &svs::cli_disconnect_viewer);
    c["disconnect_viewer"]->set_help("Disconnect from viewer.");
    
    c["filters"]           = &get_filter_table();
    c["commands"]          = &get_command_table();
    
    for (int j = 0, jend = state_stack.size(); j < jend; ++j)
    {
        c[state_stack[j]->get_name()] = state_stack[j];
    }
}

bool svs::do_cli_command(const vector<string>& args, string& output)
{
    stringstream ss;
    vector<string> rest;
    
    if (args.size() < 2)
    {
        output = "specify path\n";
        return false;
    }
    
    for (int i = 2, iend = args.size(); i < iend; ++i)
    {
        rest.push_back(args[i]);
    }
    
    proxy_use(args[1], rest, ss);
    output = ss.str();
    return true;
}

void svs::cli_connect_viewer(const vector<string>& args, ostream& os)
{
    if (args.empty())
    {
        os << "specify socket path" << endl;
        return;
    }
    if (draw->connect(args[0]))
    {
        os << "connection successful" << endl;
        for (int i = 0, iend = state_stack.size(); i < iend; ++i)
        {
            state_stack[i]->get_scene()->refresh_draw();
        }
    }
    else
    {
        os << "connection failed" << endl;
    }
}

void svs::cli_disconnect_viewer(const vector<string>& args, ostream& os)
{
    draw->disconnect();
}