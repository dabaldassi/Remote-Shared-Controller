#/usr/bin/env bash

function list() {
    local _=$(${COMP_WORDS[0]} $1 $2 2> /dev/null)
    if [ $? -eq 0 ]
    then
	OPT=$(${COMP_WORDS[0]} $1 $2 | tr '\t' ' ' | tail +$3 | cut -d" " -f$4 | tr '\n' ' ')
    fi
}

_rsccli_completions()
{
    OPT=
    
    if [ "${#COMP_WORDS[@]}" == "2" ]
    then
	COMPREPLY=($(compgen -W "help list add remove version if start stop pause shortcut set swap" -- "${COMP_WORDS[1]}"))
    elif [ "${#COMP_WORDS[@]}" == "3" ]
    then
	case "${COMP_WORDS[1]}" in
	    "list")
		OPT="-c -a -r"
		;;
	    "if")
		OPT="-s -g -l"
		;;
	    "shortcut")
		OPT="-s -l -r"
		;;
	    "set")
		OPT="circular"
		;;
	    "add")
		list list -r 2 2
		;;
	    "swap" | "remove")
		list list -c 2 2
		;;
	    *)
		return
		;;
	esac
	COMPREPLY=($(compgen -W "$OPT" -- "${COMP_WORDS[2]}"))
    elif [ "${#COMP_WORDS[@]}" == "4" ]
    then
	case "${COMP_WORDS[1]}" in
	    "set")
		OPT="disabled enabled"
		;;
	    "if")
		if [ "${COMP_WORDS[2]}" == "-s" ]
		then
		    list if -l 3 2
		fi
		;;
	    "shortcut")
		if [ "${COMP_WORDS[2]}" == "-s" -o "${COMP_WORDS[2]}" == "-l" ]
		then
		    list shortcut -l 3 1
		fi
		;;
	    "add" | "swap")
		list list -c 2 2
		;;
	    *)
		return
		;;
	esac
	COMPREPLY=($(compgen -W "$OPT" -- "${COMP_WORDS[3]}"))
    fi
}

complete -F _rsccli_completions rsccli
complete -F _rsccli_completions ./rsccli
