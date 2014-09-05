/***********************************************************************
 *
 * file filters/base_node_filter.h
 *
 * Node Test
 *  bool node_test(sgnode* a, sgnode* b, fp* p)
 *    computes a true/false test between nodes a and b
 *
 *  node_test_filter
 *    Parameters:
 *      sgnode a 
 *      sgnode b
 *    Returns:
 *      bool - output of node_test(a, b)
 *
 *  node_test_select_filter
 *    Parameters: 
 *      sgnode a
 *      sgnode b
 *    Returns:
 *      sgnode b - if node_test(a, b) is true
 *    Settings:
 *      set_select_true(bool) - whether the filter selects nodes if the test is true or false
 *
 *
 * Node Comparison
 *  double node_comparison(sgnode* a, sgnode* b, fp* p)
 *    compares the two nodes and returns a double
 *
 *  node_comparison_filter
 *    Parameters:
 *      sgnode a
 *      sgnode b
 *    Returns:
 *      double - output of node_comparison(a, b)
 *
 *  node_comparison_select_filter
 *    Parameters:
 *      sgnode a
 *      sgnode b
 *      min [Optional - defaults to -INF]
 *      max [Optional - defaults to +INF]
 *    Returns:
 *      sgnode b - if min <= node_comparison(a, b) <= max
 *    Settings:
 *      set_min(double) - change the default min value
 *      set_max(double) - change the default max value 
 *
 *  node_comparison_rank_filter
 *    Parameters:
 *      set<sgnode> a
 *      set<sgnode> b
 *    Returns:
 *      sgnode pair (a, b) from the inputs where node_comparison(a, b) is the highest
 *    Settings:
 *      set_select_highest(bool) - if false, returns the lowest node instead
 *
 *
 * Node Evaluation
 *  double node_evaluation(sgnode* a, fp* p)
 *    computes something about the node a
 *
 *  node_evaluation_filter
 *    Parameters:
 *      sgnode a
 *    Returns:
 *      double - output of node_evaluation(a, p)
 *
 *  node_evaluation_select_filter
 *    Parameters:
 *      sgnode a
 *      min [Optional - defaults to -INF]
 *      max [Optional - defaults to +INF]
 *    Returns:
 *      sgnode a - if min <= node_evaluation(a) <= max
 *    Settings:
 *      set_min(double) - change the default min value
 *      set_max(double) - change the default max value 
 *
 *  node_evaluation_rank_filter
 *    Parameters:
 *      set<sgnode> a
 *    Returns:
 *      sgnode - node from input set a where node_evaluation(a) is the highest
 *    Settings:
 *      set_select_highest(bool) - if false, returns the lowest node instead
 *
 ******************************************************************************/
#ifndef __BASE_NODE_FILTERS_H__
#define __BASE_NODE_FILTERS_H__

#include "filter.h"

/////// Node Functions ///////
typedef bool node_test(sgnode* a, sgnode* b, const filter_params* p);

typedef double node_comparison(sgnode* a, sgnode* b, const filter_params* p);

typedef double node_evaluation(sgnode* a, const filter_params* p);

/////// Node Test Filters //////
class node_test_filter : public map_filter<bool> {
  public:
    node_test_filter(Symbol* root, soar_interface* si, 
        filter_input* input, node_test* test)
      : map_filter<bool>(root, si, input), test(test)
    {}

    bool compute(const filter_params* p, bool& out);

  private:
    node_test* test;
};

class node_test_select_filter : public select_filter<sgnode*> {
  public:
    node_test_select_filter(Symbol* root, soar_interface* si, 
        filter_input* input, node_test* test)
      : select_filter<sgnode*>(root, si, input), test(test), select_true(true)
    {}

    bool compute(const filter_params* p, sgnode*& out, bool& select);

    void set_select_true(bool sel_true){
      select_true = sel_true;
    }
  private:
    node_test* test;
    bool select_true; 
};

////// Node Comparison Filters //////
class node_comparison_filter : public map_filter<double> {
  public:
    node_comparison_filter(Symbol* root, soar_interface* si, 
        filter_input* input, node_comparison* comp)
      : map_filter<double>(root, si, input), comp(comp)
    {}

    bool compute(const filter_params* p, double& out);

  private:
    node_comparison* comp;
};

class node_comparison_select_filter : public select_filter<sgnode*> {
  public:
    node_comparison_select_filter(Symbol* root, soar_interface* si, 
        filter_input* input, node_comparison* comp)
      : select_filter<sgnode*>(root, si, input), comp(comp), 
        range_min(-1000000000), range_max(1000000000)
    {}

    bool compute(const filter_params* p, sgnode*& out, bool& select);

    void set_min(bool sel_min){
      range_min = sel_min;
    }
    void set_max(bool sel_max){
      range_max = sel_max;
    }
  private:
    node_comparison* comp;
    double range_min;
    double range_max;
};

class node_comparison_rank_filter : public rank_filter {
  public:
    node_comparison_rank_filter(Symbol* root, soar_interface* si, 
        filter_input* input, node_comparison* comp)
      : rank_filter(root, si, input), comp(comp)
    {}

    bool rank(const filter_params* p, double& r);

  private:
    node_comparison* comp;
};

////// Node Evaluation Filters //////
class node_evaluation_filter : public map_filter<double> {
  public:
    node_evaluation_filter(Symbol* root, soar_interface* si, 
        filter_input* input, node_evaluation* eval)
      : map_filter<double>(root, si, input), eval(eval)
    {}

    bool compute(const filter_params* p, double& out);

  private:
    node_evaluation* eval;
};

class node_evaluation_select_filter : public select_filter<sgnode*> {
  public:
    node_evaluation_select_filter(Symbol* root, soar_interface* si, 
        filter_input* input, node_evaluation* eval)
      : select_filter<sgnode*>(root, si, input), eval(eval), 
        range_min(-1000000000), range_max(1000000000)
    {}

    bool compute(const filter_params* p, sgnode*& out, bool& select);

    void set_min(bool sel_min){
      range_min = sel_min;
    }
    void set_max(bool sel_max){
      range_max = sel_max;
    }
  private:
    node_evaluation* eval;
    double range_min;
    double range_max;
};

class node_evaluation_rank_filter : public rank_filter {
  public:
    node_evaluation_rank_filter(Symbol* root, soar_interface* si, 
        filter_input* input, node_evaluation* eval)
      : rank_filter(root, si, input), eval(eval)
    {}

    bool rank(const filter_params* p, double& r);

  private:
    node_evaluation* eval;
};

#endif //__BASE_NODE_FILTERS_H__
