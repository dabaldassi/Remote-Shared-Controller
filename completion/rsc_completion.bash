#/usr/bin/env bash

_rsc_completions()
{
    OPT=
    
    if [ "${#COMP_WORDS[@]}" == "2" ]
    then
	COMPREPLY=($(compgen -W "-i -k" -- "${COMP_WORDS[1]}"))
    elif [ "${#COMP_WORDS[@]}" == "4" ]
    then
	case "${COMP_WORDS[1]}" in
	    "-i")
		COMPREPLY=($(compgen -W "-k" -- "${COMP_WORDS[3]}"))
		;;
	    "-k")
		COMPREPLY=($(compgen -W "-i" -- "${COMP_WORDS[3]}"))
		;;
	    *)
		return
		;;
	esac
    fi
}

complete -F _rsc_completions remote-shared-controller
complete -F _rsc_completions ./remote-shared-controller
