#!/bin/sh
#
# Generates cli_help.cpp from the command documentation on the Google
# Code wiki.
#
# Wiki html -> text conversion is performed by w3m, which you can get at
# http://w3m.sourceforge.net
#
# Joseph Xu March 2011
# Modified  March 2012

if ! [ -x "`which git`" ]
then
    echo "Please install git"
    exit 1
fi

if ! [ -x "`which w3m`" ]
then
    echo "Please install w3m"
    exit 2
fi

git clone git@github.com:SoarGroup/Soar.wiki.git
mv Soar.wiki/Manuals\ and\ FAQs/CLI/ CLI/
rm -f CLI/_Sidebar.md CLI/CommandIndex.md

for file in ./CLI/*.md
do
    perl markdown.pl --html4tags $file > $file.html
    w3m -O ascii -dump -T text/html -r $file.html > $file.txt
done

pushd Soar.wiki
echo "/* Auto-generated by gen_cli_help.sh on `date` using rev `git rev-parse --short HEAD` */" > ../src/cli_help.cpp
popd
rm -rf Soar.wiki

cat cli_help_head.cpp >> src/cli_help.cpp

for f in ./CLI/*.txt
do
	c=`basename $f | cut -c 5- | rev | cut -c 8- | rev`
	awk '
		BEGIN {
			cmdname = "'$c'"
			inhead = 1
		}
		inhead == 1 {
			if (NF > 1 || (NF == 1 && $1 != cmdname)) {
				inhead = 0
				linecount = 1
			}
		}
		inhead == 0 {
			if (lines[linecount-1] ~ /^[ \t]*$/ && $0 ~ /^[ \t]*$/) {
				# skip double blanks
				next
			}
			lines[linecount++] = $0
		}
		END {
			for (i = linecount; i >= 1; i--) {
				if (lines[i] !~ /^[ \t]*$/) {
					last = i
					break
				}
			}
			print "	docstrings[\"" cmdname "\"] = "
			for (i = 1; i <= last; i++) {
				gsub("\\\\", "\\\\", lines[i])
				gsub("\"", "\\\"", lines[i])
				gsub("&#x27;", "'"'"'", lines[i])
				print "		\"" lines[i] "\\n\""
			}
			print "	;"
		}' "$f" >> src/cli_help.cpp
done

echo "}" >> src/cli_help.cpp
rm -rf CLI
