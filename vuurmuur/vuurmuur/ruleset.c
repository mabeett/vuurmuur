/***************************************************************************
 *   Copyright (C) 2002-2008 by Victor Julien                              *
 *   victor@vuurmuur.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/*  functions for creating a ruleset file that can be loaded into
    the system by iptables-restore.
*/

#include "main.h"

/* hack: in 0.8 we have to do this right! */
d_list  accounting_chain_names; /* list with the chainnames */


struct ChainRef
{
    char    chain[32];
    char    refcnt;
};


/*  ruleset_init

    Initializes the RuleSet datastructure.

    Returncodes:
         0: ok
        -1: error
*/
static int
ruleset_setup(const int debuglvl, RuleSet *ruleset)
{
    /* safety */
    if(ruleset == NULL)
    {
        (void)vrprint.error(-1, "Internal Error", "parameter problem (in: %s:%d).",
                                    __FUNC__, __LINE__);
        return(-1);
    }

    /* init */
    memset(ruleset, 0, sizeof(RuleSet));

    /* init the lists */

    /* mangle */
    if(d_list_setup(debuglvl, &ruleset->mangle_preroute, free) < 0)
        return(-1);
    if(d_list_setup(debuglvl, &ruleset->mangle_input, free) < 0)
        return(-1);
    if(d_list_setup(debuglvl, &ruleset->mangle_forward, free) < 0)
        return(-1);
    if(d_list_setup(debuglvl, &ruleset->mangle_output, free) < 0)
        return(-1);
    if(d_list_setup(debuglvl, &ruleset->mangle_postroute, free) < 0)
        return(-1);

    if(d_list_setup(debuglvl, &ruleset->mangle_shape_in, free) < 0)
        return(-1);
    if(d_list_setup(debuglvl, &ruleset->mangle_shape_out, free) < 0)
        return(-1);
    if(d_list_setup(debuglvl, &ruleset->mangle_shape_fw, free) < 0)
        return(-1);

    /* nat */
    if(d_list_setup(debuglvl, &ruleset->nat_preroute, free) < 0)
        return(-1);
    if(d_list_setup(debuglvl, &ruleset->nat_postroute, free) < 0)
        return(-1);
    if(d_list_setup(debuglvl, &ruleset->nat_output, free) < 0)
        return(-1);

    /* filter */
    if(d_list_setup(debuglvl, &ruleset->filter_input, free) < 0)
        return(-1);
    if(d_list_setup(debuglvl, &ruleset->filter_forward, free) < 0)
        return(-1);
    if(d_list_setup(debuglvl, &ruleset->filter_output, free) < 0)
        return(-1);

    if(d_list_setup(debuglvl, &ruleset->filter_antispoof, free) < 0)
        return(-1);
    if(d_list_setup(debuglvl, &ruleset->filter_blocklist, free) < 0)
        return(-1);
    if(d_list_setup(debuglvl, &ruleset->filter_blocktarget, free) < 0)
        return(-1);
    if(d_list_setup(debuglvl, &ruleset->filter_badtcp, free) < 0)
        return(-1);
    if(d_list_setup(debuglvl, &ruleset->filter_synlimittarget, free) < 0)
        return(-1);
    if(d_list_setup(debuglvl, &ruleset->filter_udplimittarget, free) < 0)
        return(-1);
    /* NFQueue state */
    if(d_list_setup(debuglvl, &ruleset->filter_newnfqueuetarget, free) < 0)
        return(-1);
    if(d_list_setup(debuglvl, &ruleset->filter_estrelnfqueuetarget, free) < 0)
        return(-1);
    /* tcp reset */
    if(d_list_setup(debuglvl, &ruleset->filter_tcpresettarget, free) < 0)
        return(-1);
    /* accounting */
    if(d_list_setup(debuglvl, &ruleset->filter_accounting, free) < 0)
        return(-1);
    if(d_list_setup(debuglvl, &accounting_chain_names, free) < 0)
        return(-1);

    /* shaping */
    if(d_list_setup(debuglvl, &ruleset->tc_rules, free) < 0)
        return(-1);

    return(0);
}


/*  cleanup the ruleset

    All lists are cleaned.

    Returns:
        nothing, void function
*/
static void
ruleset_cleanup(const int debuglvl, RuleSet *ruleset)
{
    /* safety */
    if(ruleset == NULL)
    {
        (void)vrprint.error(-1, "Internal Error", "parameter problem "
            "(in: %s:%d).", __FUNC__, __LINE__);
        return;
    }

    /* mangle */
    d_list_cleanup(debuglvl, &ruleset->mangle_preroute);
    d_list_cleanup(debuglvl, &ruleset->mangle_input);
    d_list_cleanup(debuglvl, &ruleset->mangle_forward);
    d_list_cleanup(debuglvl, &ruleset->mangle_output);
    d_list_cleanup(debuglvl, &ruleset->mangle_postroute);

    d_list_cleanup(debuglvl, &ruleset->mangle_shape_in);
    d_list_cleanup(debuglvl, &ruleset->mangle_shape_out);
    d_list_cleanup(debuglvl, &ruleset->mangle_shape_fw);

    /* nat */
    d_list_cleanup(debuglvl, &ruleset->nat_preroute);
    d_list_cleanup(debuglvl, &ruleset->nat_postroute);
    d_list_cleanup(debuglvl, &ruleset->nat_output);

    /* filter */
    d_list_cleanup(debuglvl, &ruleset->filter_input);
    d_list_cleanup(debuglvl, &ruleset->filter_forward);
    d_list_cleanup(debuglvl, &ruleset->filter_output);

    d_list_cleanup(debuglvl, &ruleset->filter_antispoof);
    d_list_cleanup(debuglvl, &ruleset->filter_blocklist);
    d_list_cleanup(debuglvl, &ruleset->filter_blocktarget);
    d_list_cleanup(debuglvl, &ruleset->filter_badtcp);
    d_list_cleanup(debuglvl, &ruleset->filter_synlimittarget);
    d_list_cleanup(debuglvl, &ruleset->filter_udplimittarget);
    d_list_cleanup(debuglvl, &ruleset->filter_estrelnfqueuetarget);
    d_list_cleanup(debuglvl, &ruleset->filter_newnfqueuetarget);
    d_list_cleanup(debuglvl, &ruleset->filter_tcpresettarget);

    d_list_cleanup(debuglvl, &ruleset->filter_accounting);
    d_list_cleanup(debuglvl, &accounting_chain_names);

    d_list_cleanup(debuglvl, &ruleset->tc_rules);

    /* clear all memory */
    memset(ruleset, 0, sizeof(RuleSet));
}


/*

    returncodes:
         1: ok, create
         0: ok, don't create the acc rule
        -1: error
*/
static int
ruleset_check_accounting(const int debuglvl, char *chain)
{
    d_list_node     *d_node = NULL;
    char            chain_found = 0;
    char            stripped_chain[32] = "",
                    commandline_switch[3] = ""; /* '-A' =2 + '\0' = 1 == 3 */
    struct ChainRef *chainref_ptr = NULL;

    /* safety */
    if(!chain)
    {
        (void)vrprint.error(-1, "Internal Error", "parameter problem (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /* accouting rule check */
    if(strncmp(chain, "-A ACC-", 7) == 0)
    {
        /* strip chain from -A */
        sscanf(chain, "%3s %32s", commandline_switch, stripped_chain);
        if(debuglvl >= HIGH)
            (void)vrprint.debug(__FUNC__, "chain: '%s', commandline_switch: '%s', stripped_chain '%s'.",
                                chain,
                                commandline_switch,
                                stripped_chain);

        /*  okay, this is a accounting rule. Accounting rules have
            dynamic chain names, so lets see if we already know this chain.
        */
        for(d_node = accounting_chain_names.top; d_node; d_node = d_node->next)
        {
            if(!(chainref_ptr = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            if(strcmp(chainref_ptr->chain, stripped_chain) == 0)
            {
                if(debuglvl >= HIGH)
                    (void)vrprint.debug(__FUNC__, "chain '%s' already in the list.", chainref_ptr->chain);

                chain_found = 1;
                break;
            }
        }

        if(!chain_found)
        {
            if(debuglvl >= HIGH)
                (void)vrprint.debug(__FUNC__, "going to add chain '%s' to the list.", stripped_chain);

            /* okay, lets add the chain name to the list */
            //size = strlen(stripped_chain) + 1;

            /* alloc since the chain name will not be a pointer to a static buffer */
            if(!(chainref_ptr = malloc(sizeof(struct ChainRef))))
            {
                (void)vrprint.error(-1, "Error", "malloc failed: %s (in: %s:%d).", strerror(errno), __FUNC__, __LINE__);
                return(-1);
            }

            /* copy */
            (void)strlcpy(chainref_ptr->chain, stripped_chain, sizeof(chainref_ptr->chain));
            chainref_ptr->refcnt = 1;

            if(debuglvl >= HIGH)
                (void)vrprint.debug(__FUNC__, "appending chain '%s' to the list.", chainref_ptr->chain);

            /* append to the list */
            if(d_list_append(debuglvl, &accounting_chain_names, chainref_ptr) == NULL)
            {
                (void)vrprint.error(-1, "Internal Error", "appending rule to list failed (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }
        }
        else
        {
            if(chainref_ptr->refcnt > 1)
            {
                if(debuglvl >= HIGH)
                    (void)vrprint.debug(__FUNC__, "already 2 rules created in '%s'.", chainref_ptr->chain);

                return(0);
            }
            else
            {
                chainref_ptr->refcnt++;
            }
        }
    }

    return(1);
}


/*  ruleset_add_rule_to_set

    Add a iptables-restore compatible string 'line' to the ruleset.
    
    Note: this function will alloc the string and copy rule to this
    string.

    Returncodes:
         0: ok
        -1: error
*/
int
ruleset_add_rule_to_set(const int debuglvl, d_list *list, char *chain, char *rule, unsigned long long packets, unsigned long long bytes)
{
    size_t  size = 0,
            numbers_size = 0;
    char    *line = NULL,
            numbers[32] = "";
    int     result = 0;

    /* safety */
    if(!list || !chain || !rule)
    {
        (void)vrprint.error(-1, "Internal Error", "parameter problem (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /* HACK: check for accounting special cases */
    result = ruleset_check_accounting(debuglvl, chain);
    if(result == -1)
        return(-1);
    else if(result == 0)
        return(0);

    /* create the counters */
    if(packets > 0 || bytes > 0)
    {
        snprintf(numbers, sizeof(numbers), "[%llu:%llu] ", packets, bytes);
        numbers_size = strlen(numbers);
    }

    /* size of the numbers string, chain, space, rule */
    size = numbers_size + strlen(chain) + 1 + strlen(rule) + 1;
    if(size == 0)
    {
        (void)vrprint.error(-1, "Internal Error", "cannot append an empty string (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /* alloc the size for line */
    if(!(line = malloc(size)))
    {
        (void)vrprint.error(-1, "Error", "malloc failed: %s (in: %s:%d).", strerror(errno), __FUNC__, __LINE__);
        return(-1);
    }

    /* create the string */
    result = snprintf(line, size, "%s%s %s", numbers, chain, rule);
    if(result >= (int)size)
    {
        (void)vrprint.error(-1, "Error", "ruleset string overflow (%d >= %d, %s) (in: %s:%d).", result, size, rule, __FUNC__, __LINE__);
        return(-1);
    }

    /* append to the list */
    if(d_list_append(debuglvl, list, line) == NULL)
    {
        (void)vrprint.error(-1, "Internal Error", "appending rule to list failed (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    return(0);
}


/*  ruleset_writeprint

    wrapper around the write call
*/
static int
ruleset_writeprint(const int fd, const char *line)
{
    return((int)write(fd, line, strlen(line)));
}


/* Create the shaping script file */
static int
ruleset_fill_shaping_file(const int debuglvl, RuleSet *ruleset, int fd) {
    d_list_node *d_node = NULL;
    char        *ptr = NULL;
    char        cmd[MAX_PIPE_COMMAND] = "";

    ruleset_writeprint(fd, "#!/bin/bash\n");

    for (d_node = ruleset->tc_rules.top; d_node; d_node = d_node->next) {
        ptr = d_node->data;

        snprintf(cmd, sizeof(cmd), "%s\n", ptr);
        ruleset_writeprint(fd, cmd);
    }
    
    ruleset_writeprint(fd, "# EOF\n");

    return(0);
}

/*  ruleset_create_file

    Creates the ruleset file to be loaded by iptables-restore

    Returncodes:
         0: ok
        -1: error
*/
static int
ruleset_fill_file(  const int debuglvl,
                    Rules *rules,
                    RuleSet *ruleset,
                    IptCap *iptcap,
                    int ruleset_fd,
                    struct vuurmuur_config *cnf)
{
    d_list_node *d_node = NULL;
    char        *rule = NULL,
                *cname = NULL;
    char        cmd[512] = "";

    /* safety */
    if(ruleset == NULL || cnf == NULL || iptcap == NULL)
    {
        (void)vrprint.error(-1, "Internal Error", "parameter problem (in: %s:%d).",
                                        __FUNC__, __LINE__);
        return(-1);
    }

    /* get the current chains */
    (void)rules_get_system_chains(debuglvl, rules, cnf);

    snprintf(cmd, sizeof(cmd), "# Generated by Vuurmuur %s (c) 2002-2008 Victor Julien\n", version_string);
    ruleset_writeprint(ruleset_fd, cmd);
    snprintf(cmd, sizeof(cmd), "# DO NOT EDIT: file will be overwritten.\n");
    ruleset_writeprint(ruleset_fd, cmd);

    if(cnf->check_iptcaps == FALSE || iptcap->table_mangle == TRUE)
    {
        /* first process the mangle table */
        snprintf(cmd, sizeof(cmd), "*mangle\n");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), ":PREROUTING %s [0:0]\n", ruleset->mangle_preroute_policy ? "DROP" : "ACCEPT");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), ":INPUT %s [0:0]\n", ruleset->mangle_input_policy ? "DROP" : "ACCEPT");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), ":FORWARD %s [0:0]\n", ruleset->mangle_forward_policy ? "DROP" : "ACCEPT");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), ":OUTPUT %s [0:0]\n", ruleset->mangle_output_policy ? "DROP" : "ACCEPT");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), ":POSTROUTING %s [0:0]\n", ruleset->mangle_postroute_policy ? "DROP" : "ACCEPT");
        ruleset_writeprint(ruleset_fd, cmd);

        /*
            BEGIN -- PRE-VUURMUUR-CHAINS feature - by(as).
            Allow to make some specials rules before the Vuurmuur rules kick in.

            Only create if they don't exist.
            No flushing, the content of these chains is the responsibility of the user.
        */

        /* mangle table uses {PREROUTING,INPUT,FORWARD,POSTROUTING,OUTPUT} hooks */

        if(!rules_chain_in_list(debuglvl, &rules->system_chain_mangle, "PRE-VRMR-PREROUTING"))
        {
            snprintf(cmd, sizeof(cmd), "--new PRE-VRMR-PREROUTING\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        if(!rules_chain_in_list(debuglvl, &rules->system_chain_mangle, "PRE-VRMR-INPUT"))
        {
            snprintf(cmd, sizeof(cmd), "--new PRE-VRMR-INPUT\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        if(!rules_chain_in_list(debuglvl, &rules->system_chain_mangle, "PRE-VRMR-FORWARD"))
        {
            snprintf(cmd, sizeof(cmd), "--new PRE-VRMR-FORWARD\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        if(!rules_chain_in_list(debuglvl, &rules->system_chain_mangle, "PRE-VRMR-POSTROUTING"))
        {
            snprintf(cmd, sizeof(cmd), "--new PRE-VRMR-POSTROUTING\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        if(!rules_chain_in_list(debuglvl, &rules->system_chain_mangle, "PRE-VRMR-OUTPUT"))
        {
            snprintf(cmd, sizeof(cmd), "--new PRE-VRMR-OUTPUT\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }

        /* END -- PRE-VUURMUUR-CHAINS feature - by(as). */

        snprintf(cmd, sizeof(cmd), "--flush PREROUTING\n");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), "--flush INPUT\n");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), "--flush FORWARD\n");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), "--flush OUTPUT\n");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), "--flush POSTROUTING\n");
        ruleset_writeprint(ruleset_fd, cmd);

        /* SHAPE IN */
        if(rules_chain_in_list(debuglvl, &rules->system_chain_mangle, "SHAPEIN"))
        {
            snprintf(cmd, sizeof(cmd), "--flush SHAPEIN\n");
            ruleset_writeprint(ruleset_fd, cmd);
            snprintf(cmd, sizeof(cmd), "--delete-chain SHAPEIN\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        snprintf(cmd, sizeof(cmd), "--new SHAPEIN\n");
        ruleset_writeprint(ruleset_fd, cmd);

        /* SHAPE OUT */
        if(rules_chain_in_list(debuglvl, &rules->system_chain_mangle, "SHAPEOUT"))
        {
            snprintf(cmd, sizeof(cmd), "--flush SHAPEOUT\n");
            ruleset_writeprint(ruleset_fd, cmd);
            snprintf(cmd, sizeof(cmd), "--delete-chain SHAPEOUT\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        snprintf(cmd, sizeof(cmd), "--new SHAPEOUT\n");
        ruleset_writeprint(ruleset_fd, cmd);

        /* SHAPE FW */
        if(rules_chain_in_list(debuglvl, &rules->system_chain_mangle, "SHAPEFW"))
        {
            snprintf(cmd, sizeof(cmd), "--flush SHAPEFW\n");
            ruleset_writeprint(ruleset_fd, cmd);
            snprintf(cmd, sizeof(cmd), "--delete-chain SHAPEFW\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        snprintf(cmd, sizeof(cmd), "--new SHAPEFW\n");
        ruleset_writeprint(ruleset_fd, cmd);

        /* prerouting */
        for(d_node = ruleset->mangle_preroute.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }
        /* input */
        for(d_node = ruleset->mangle_input.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }
        /* forward */
        for(d_node = ruleset->mangle_forward.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }
        /* output */
        for(d_node = ruleset->mangle_output.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }
        /* postrouting */
        for(d_node = ruleset->mangle_postroute.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }

        /* shape in */
        for(d_node = ruleset->mangle_shape_in.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }

        /* shape out */
        for(d_node = ruleset->mangle_shape_out.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }

        /* shape fw */
        for(d_node = ruleset->mangle_shape_fw.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }

        snprintf(cmd, sizeof(cmd), "COMMIT\n");
        ruleset_writeprint(ruleset_fd, cmd);
    }

    if(cnf->check_iptcaps == FALSE || iptcap->table_nat == TRUE)
    {
        /* nat table */
        snprintf(cmd, sizeof(cmd), "*nat\n");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), ":PREROUTING %s [0:0]\n", ruleset->nat_preroute_policy ? "DROP" : "ACCEPT");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), ":OUTPUT %s [0:0]\n", ruleset->nat_output_policy ? "DROP" : "ACCEPT");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), ":POSTROUTING %s [0:0]\n", ruleset->nat_postroute_policy ? "DROP" : "ACCEPT");
        ruleset_writeprint(ruleset_fd, cmd);

        /*
            BEGIN -- PRE-VUURMUUR-CHAINS feature - by(as).
            Allow to make some specials rules before the Vuurmuur rules kick in.

            Only create if they don't exist.
            No flushing, the content of these chains is the responsibility of the user.
        */

        /* nat table uses {PREROUTING,POSTROUTING,OUTPUT} hooks */

        if(!rules_chain_in_list(debuglvl, &rules->system_chain_nat, "PRE-VRMR-PREROUTING"))
        {
            snprintf(cmd, sizeof(cmd), "--new PRE-VRMR-PREROUTING\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        if(!rules_chain_in_list(debuglvl, &rules->system_chain_nat, "PRE-VRMR-POSTROUTING"))
        {
            snprintf(cmd, sizeof(cmd), "--new PRE-VRMR-POSTROUTING\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        if(!rules_chain_in_list(debuglvl, &rules->system_chain_nat, "PRE-VRMR-OUTPUT"))
        {
            snprintf(cmd, sizeof(cmd), "--new PRE-VRMR-OUTPUT\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }

        /* END -- PRE-VUURMUUR-CHAINS feature - by(as). */

        snprintf(cmd, sizeof(cmd), "--flush PREROUTING\n");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), "--flush OUTPUT\n");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), "--flush POSTROUTING\n");
        ruleset_writeprint(ruleset_fd, cmd);

        /* prerouting */
        for(d_node = ruleset->nat_preroute.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }
        /* output */
        for(d_node = ruleset->nat_output.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }
        /* postrouting */
        for(d_node = ruleset->nat_postroute.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }

        snprintf(cmd, sizeof(cmd), "COMMIT\n");
        ruleset_writeprint(ruleset_fd, cmd);
    }

    if(cnf->check_iptcaps == FALSE || iptcap->table_filter == TRUE)
    {
        /* finally the filter table */
        snprintf(cmd, sizeof(cmd), "*filter\n");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), ":INPUT %s [0:0]\n", ruleset->filter_input_policy ? "DROP" : "ACCEPT");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), ":FORWARD %s [0:0]\n", ruleset->filter_forward_policy ? "DROP" : "ACCEPT");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), ":OUTPUT %s [0:0]\n", ruleset->filter_output_policy ? "DROP" : "ACCEPT");
        ruleset_writeprint(ruleset_fd, cmd);

        snprintf(cmd, sizeof(cmd), "--flush INPUT\n");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), "--flush FORWARD\n");
        ruleset_writeprint(ruleset_fd, cmd);
        snprintf(cmd, sizeof(cmd), "--flush OUTPUT\n");
        ruleset_writeprint(ruleset_fd, cmd);


        /*
            BEGIN -- PRE-VUURMUUR-CHAINS feature - by(as).
            Allow to make some specials rules before the Vuurmuur rules kick in.

            Only create if they don't exist.
            No flushing, the content of these chains is the responsibility of the user.
        */

        /* filter table uses {INPUT,FORWARD,OUTPUT} hooks */

        if(!rules_chain_in_list(debuglvl, &rules->system_chain_filter, "PRE-VRMR-INPUT"))
        {
            snprintf(cmd, sizeof(cmd), "--new PRE-VRMR-INPUT\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        if(!rules_chain_in_list(debuglvl, &rules->system_chain_filter, "PRE-VRMR-FORWARD"))
        {
            snprintf(cmd, sizeof(cmd), "--new PRE-VRMR-FORWARD\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        if(!rules_chain_in_list(debuglvl, &rules->system_chain_filter, "PRE-VRMR-OUTPUT"))
        {
            snprintf(cmd, sizeof(cmd), "--new PRE-VRMR-OUTPUT\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }

        /* END -- PRE-VUURMUUR-CHAINS feature - by(as). */

        /* create the custom chains, because some rules will depend on them */
        for(d_node = rules->custom_chain_list.top; d_node; d_node = d_node->next)
        {
            if(!(cname = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).",
                                        __FUNC__, __LINE__);
                return(-1);
            }

            if(!rules_chain_in_list(debuglvl, &rules->system_chain_filter, cname))
            {
                snprintf(cmd, sizeof(cmd), "--new %s\n", cname);
                ruleset_writeprint(ruleset_fd, cmd);
            }
        }

        if(rules_chain_in_list(debuglvl, &rules->system_chain_filter, "ANTISPOOF"))
        {
            snprintf(cmd, sizeof(cmd), "--flush ANTISPOOF\n");
            ruleset_writeprint(ruleset_fd, cmd);
            snprintf(cmd, sizeof(cmd), "--delete-chain ANTISPOOF\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        snprintf(cmd, sizeof(cmd), "--new ANTISPOOF\n");
        ruleset_writeprint(ruleset_fd, cmd);

        if(rules_chain_in_list(debuglvl, &rules->system_chain_filter, "BLOCKLIST"))
        {
            snprintf(cmd, sizeof(cmd), "--flush BLOCKLIST\n");
            ruleset_writeprint(ruleset_fd, cmd);
            snprintf(cmd, sizeof(cmd), "--delete-chain BLOCKLIST\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        snprintf(cmd, sizeof(cmd), "--new BLOCKLIST\n");
        ruleset_writeprint(ruleset_fd, cmd);

        if(rules_chain_in_list(debuglvl, &rules->system_chain_filter, "BLOCK"))
        {
            snprintf(cmd, sizeof(cmd), "--flush BLOCK\n");
            ruleset_writeprint(ruleset_fd, cmd);
            snprintf(cmd, sizeof(cmd), "--delete-chain BLOCK\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        snprintf(cmd, sizeof(cmd), "--new BLOCK\n");
        ruleset_writeprint(ruleset_fd, cmd);

        /* do NEWACCEPT and NEWQUEUE before SYNLIMIT and UDPLIMIT */
        if(rules_chain_in_list(debuglvl, &rules->system_chain_filter, "NEWACCEPT"))
        {
            snprintf(cmd, sizeof(cmd), "--flush NEWACCEPT\n");
            ruleset_writeprint(ruleset_fd, cmd);
            snprintf(cmd, sizeof(cmd), "--delete-chain NEWACCEPT\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        snprintf(cmd, sizeof(cmd), "--new NEWACCEPT\n");
        ruleset_writeprint(ruleset_fd, cmd);

        if(rules_chain_in_list(debuglvl, &rules->system_chain_filter, "NEWQUEUE"))
        {
            snprintf(cmd, sizeof(cmd), "--flush NEWQUEUE\n");
            ruleset_writeprint(ruleset_fd, cmd);
            snprintf(cmd, sizeof(cmd), "--delete-chain NEWQUEUE\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        snprintf(cmd, sizeof(cmd), "--new NEWQUEUE\n");
        ruleset_writeprint(ruleset_fd, cmd);

        /* Do this before NEWNFQUEUE because it references
         * to it. */
        if(rules_chain_in_list(debuglvl, &rules->system_chain_filter, "ESTRELNFQUEUE"))
        {
            snprintf(cmd, sizeof(cmd), "--flush ESTRELNFQUEUE\n");
            ruleset_writeprint(ruleset_fd, cmd);
            snprintf(cmd, sizeof(cmd), "--delete-chain ESTRELNFQUEUE\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        snprintf(cmd, sizeof(cmd), "--new ESTRELNFQUEUE\n");
        ruleset_writeprint(ruleset_fd, cmd);

        if(rules_chain_in_list(debuglvl, &rules->system_chain_filter, "NEWNFQUEUE"))
        {
            snprintf(cmd, sizeof(cmd), "--flush NEWNFQUEUE\n");
            ruleset_writeprint(ruleset_fd, cmd);
            snprintf(cmd, sizeof(cmd), "--delete-chain NEWNFQUEUE\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        snprintf(cmd, sizeof(cmd), "--new NEWNFQUEUE\n");
        ruleset_writeprint(ruleset_fd, cmd);

        if(rules_chain_in_list(debuglvl, &rules->system_chain_filter, "SYNLIMIT"))
        {
            snprintf(cmd, sizeof(cmd), "--flush SYNLIMIT\n");
            ruleset_writeprint(ruleset_fd, cmd);
            snprintf(cmd, sizeof(cmd), "--delete-chain SYNLIMIT\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        snprintf(cmd, sizeof(cmd), "--new SYNLIMIT\n");
        ruleset_writeprint(ruleset_fd, cmd);

        if(rules_chain_in_list(debuglvl, &rules->system_chain_filter, "UDPLIMIT"))
        {
            snprintf(cmd, sizeof(cmd), "--flush UDPLIMIT\n");
            ruleset_writeprint(ruleset_fd, cmd);
            snprintf(cmd, sizeof(cmd), "--delete-chain UDPLIMIT\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        snprintf(cmd, sizeof(cmd), "--new UDPLIMIT\n");
        ruleset_writeprint(ruleset_fd, cmd);

        if(rules_chain_in_list(debuglvl, &rules->system_chain_filter, "TCPRESET"))
        {
            snprintf(cmd, sizeof(cmd), "--flush TCPRESET\n");
            ruleset_writeprint(ruleset_fd, cmd);
            snprintf(cmd, sizeof(cmd), "--delete-chain TCPRESET\n");
            ruleset_writeprint(ruleset_fd, cmd);
        }
        snprintf(cmd, sizeof(cmd), "--new TCPRESET\n");
        ruleset_writeprint(ruleset_fd, cmd);

        /* finally the accounting chains */
        for(d_node = accounting_chain_names.top; d_node; d_node = d_node->next)
        {
            if(!(cname = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            if(rules_chain_in_list(debuglvl, &rules->system_chain_filter, cname))
            {
                snprintf(cmd, sizeof(cmd), "--flush %s\n", cname);
                ruleset_writeprint(ruleset_fd, cmd);
                snprintf(cmd, sizeof(cmd), "--delete-chain %s\n", cname);
                ruleset_writeprint(ruleset_fd, cmd);
            }
            snprintf(cmd, sizeof(cmd), "--new %s\n", cname);
            ruleset_writeprint(ruleset_fd, cmd);
        }

        /* input */
        for(d_node = ruleset->filter_input.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }
        /* forward */
        for(d_node = ruleset->filter_forward.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }
        /* output */
        for(d_node = ruleset->filter_output.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }

        /* antispoof */
        for(d_node = ruleset->filter_antispoof.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }
        /* blocklist */
        for(d_node = ruleset->filter_blocklist.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }
        /* block */
        for(d_node = ruleset->filter_blocktarget.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }
        /* synlimit */
        for(d_node = ruleset->filter_synlimittarget.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }
        /* udplimit */
        for(d_node = ruleset->filter_udplimittarget.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }
        /* newaccept */
        for(d_node = ruleset->filter_newaccepttarget.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }
        /* newqueue */
        for(d_node = ruleset->filter_newqueuetarget.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }
        /* newnfqueue */
        for(d_node = ruleset->filter_newnfqueuetarget.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }
        /* estrelnfqueue */
        for(d_node = ruleset->filter_estrelnfqueuetarget.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }

        /* tcpreset */
        for(d_node = ruleset->filter_tcpresettarget.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }

        /* accounting */
        for(d_node = ruleset->filter_accounting.top; d_node; d_node = d_node->next)
        {
            if(!(rule = d_node->data))
            {
                (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).",
                                            __FUNC__, __LINE__);
                return(-1);
            }

            snprintf(cmd, sizeof(cmd), "%s\n", rule);
            ruleset_writeprint(ruleset_fd, cmd);
        }


        snprintf(cmd, sizeof(cmd), "COMMIT\n");
        ruleset_writeprint(ruleset_fd, cmd);
    }

    snprintf(cmd, sizeof(cmd), "# Completed\n");
    ruleset_writeprint(ruleset_fd, cmd);

    /* list of chains in the system */
    d_list_cleanup(debuglvl, &rules->system_chain_filter);
    d_list_cleanup(debuglvl, &rules->system_chain_mangle);
    d_list_cleanup(debuglvl, &rules->system_chain_nat);
    //d_list_cleanup(debuglvl, &rules->system_chain_raw);

    return(0);
}


/*  ruleset_exists

    Returncodes:
        1: exists
        0: don't exist
*/
static int
ruleset_exists(const int debuglvl, char *path_to_ruleset)
{
    FILE    *fp = NULL;

    /* safety */
    if(!path_to_ruleset)
    {
        (void)vrprint.error(-1, "Internal Error", "parameter problem (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /* check if we can read the file */
    if(!(fp = fopen(path_to_ruleset, "r")))
        return(0);

    fclose(fp);
    return(1);
}


/*  ruleset_load_ruleset

    Actually loads the ruleset
    
    Returncodes:
        -1: error
         0: ok
*/
static int
ruleset_load_ruleset(const int debuglvl, char *path_to_ruleset, char *path_to_resultfile, struct vuurmuur_config *cnf)
{
    char cmd[256] = "";

    /* safety */
    if(!path_to_ruleset || !cnf)
    {
        (void)vrprint.error(-1, "Internal Error", "parameter problem (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /*
        final checks on the file
    */

    /* exist */
    if(!(ruleset_exists(debuglvl, path_to_ruleset)))
    {
        (void)vrprint.error(-1, "Error", "missing rulesetfile (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /* stat_ok */
    if(!(stat_ok(debuglvl, path_to_ruleset, STATOK_WANT_FILE, STATOK_VERBOSE)))
    {
        (void)vrprint.error(-1, "Error", "serious file problem (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /*
        create and execute the command
    */
    if(snprintf(cmd, sizeof(cmd), "%s  --counters --noflush < %s 2>> %s",
        cnf->iptablesrestore_location, path_to_ruleset,
        path_to_resultfile) >= (int)sizeof(cmd))
    {
        (void)vrprint.error(-1, "Error", "command string overflow (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /* all good so far, lets load the ruleset */
    if(pipe_command(debuglvl, cnf, cmd, PIPE_VERBOSE) < 0)
    {
        (void)vrprint.error(-1, "Error", "loading the ruleset failed (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    return(0);
}

/*  ruleset_load_shape_ruleset

    Actually loads the shape ruleset
    
    Returncodes:
        -1: error
         0: ok
*/
static int
ruleset_load_shape_ruleset(const int debuglvl, char *path_to_ruleset, char *path_to_resultfile, struct vuurmuur_config *cnf)
{
    char    cmd[256] = "";

    /* safety */
    if(!path_to_ruleset || !cnf)
    {
        (void)vrprint.error(-1, "Internal Error", "parameter problem (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /*
        final checks on the file
    */

    /* exist */
    if(!(ruleset_exists(debuglvl, path_to_ruleset)))
    {
        (void)vrprint.error(-1, "Error", "missing rulesetfile (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /* stat_ok */
    if(!(stat_ok(debuglvl, path_to_ruleset, STATOK_WANT_FILE, STATOK_VERBOSE)))
    {
        (void)vrprint.error(-1, "Error", "serious file problem (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /*
        create and execute the REAL command
    */

    /* */
    if(snprintf(cmd, sizeof(cmd), "/bin/bash %s 2>> %s", path_to_ruleset,
        path_to_resultfile) >= (int)sizeof(cmd))
    {
        (void)vrprint.error(-1, "Error", "command string overflow (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /* all good so far, lets load the ruleset */
    if(pipe_command(debuglvl, cnf, cmd, PIPE_VERBOSE) < 0)
    {
        (void)vrprint.error(-1, "Error", "loading the shape ruleset failed (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    return(0);
}


/*  ruleset_create_ruleset

    fills the ruleset structure

    Returncodes:
         0: ok
        -1: error
*/
static int
ruleset_create_ruleset( const int debuglvl,
                        RuleSet *ruleset,
                        Rules *rules,
                        Zones *zones,
                        Interfaces *interfaces,
                        Services *services,
                        BlockList *blocklist,
                        IptCap *iptcap,
                        struct vuurmuur_config *cnf)
{
    int     result = 0;
    char    forward_rules = 0;

    /* safety */
    if(!ruleset || !rules || !zones || !interfaces || !services || !iptcap || !cnf || !blocklist)
    {
        (void)vrprint.error(-1, "Internal Error", "parameter problem (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /* create shaping setup */
    if(shaping_clear_interfaces(debuglvl, cnf, interfaces, ruleset) < 0)
    {
        (void)vrprint.error(-1, "Error", "setting up interface shaping clearing failed (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }
    if(shaping_setup_roots(debuglvl, cnf, interfaces, ruleset) < 0)
    {
        (void)vrprint.error(-1, "Error", "setting up interface shaping roots failed (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }
    if(shaping_create_default_rules(debuglvl, cnf, interfaces, ruleset) < 0)
    {
        (void)vrprint.error(-1, "Error", "setting up interface default rules failed (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }


    (void)vrprint.info("Info", "Creating the rules... (rules to create: %d)", rules->list.len);

    /* create the prerules if were called with it */
    result = pre_rules(debuglvl, ruleset, interfaces, iptcap);
    if(result < 0)
    {
        (void)vrprint.error(-1, "Error", "create pre-rules failed.");
        return(-1);
    }

    /* create NEWNFQUEUE target */
    if(create_newnfqueue_rules(debuglvl, ruleset, rules) < 0)
    {
        (void)vrprint.error(-1, "Error", "create newnfqueue failed.");
    }
    /* NFQUEUE related established */
    if(create_estrelnfqueue_rules(debuglvl, ruleset, rules) < 0)
    {
        (void)vrprint.error(-1, "Error", "create estrelnfqueue failed.");
    }

    /* create the blocklist */
    if(create_block_rules(debuglvl, ruleset, blocklist) < 0)
    {
        (void)vrprint.error(-1, "Error", "create blocklist failed.");
    }

    /* create the interface rules */
    if(create_interface_rules(debuglvl, ruleset, interfaces) < 0)
    {
        (void)vrprint.error(-1, "Error", "create protectrules failed.");
    }
    /* create the network protect rules (anti-spoofing) */
    if(create_network_protect_rules(debuglvl, ruleset, zones, iptcap) < 0)
    {
        (void)vrprint.error(-1, "Error", "create protectrules failed.");
    }
    /* system protect rules (proc) */
    if(create_system_protectrules(debuglvl, &conf) < 0)
    {
        (void)vrprint.error(-1, "Error", "create protectrules failed.");
    }
    /* normal rules, ruleset == NULL */
    if(create_normal_rules(debuglvl, cnf, ruleset, rules, interfaces, iptcap, &forward_rules) < 0)
    {
        (void)vrprint.error(-1, "Error", "create normal rules failed.");
    }

    /* post rules: enable logging */
    if(post_rules(debuglvl, ruleset, iptcap, forward_rules) < 0)
        return(-1);

    (void)vrprint.info("Info", "Creating rules finished.");
    return(0);
}


static int
ruleset_save_interface_counters(const int debuglvl, Interfaces *interfaces)
{
    d_list_node             *d_node = NULL;
    struct InterfaceData_   *iface_ptr = NULL;
    unsigned long long      tmp_ull = 0;
    char                    acc_chain[32] = "";

    /* safety */
    if(!interfaces)
    {
        (void)vrprint.error(-1, "Internal Error", "parameter problem (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /* loop through the interfaces */
    for(d_node = interfaces->list.top; d_node; d_node = d_node->next)
    {
        if(!(iface_ptr = d_node->data))
        {
            (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
            return(-1);
        }

        /* Check for empty device string and virtual device. */
        if(strcmp(iface_ptr->device, "") != 0 && !iface_ptr->device_virtual)
        {
            if(iface_ptr->cnt == NULL)
            {
                /* alloc the counters */
                if(!(iface_ptr->cnt = malloc(sizeof(InterfaceCount))))
                {
                    (void)vrprint.error(-1, "Error", "malloc failed: %s (in: %s:%d).", strerror(errno), __FUNC__, __LINE__);
                    return(-1);
                }
            }
            memset(iface_ptr->cnt, 0, sizeof(InterfaceCount));

            /* get the real counters from iptables */
            get_iface_stats_from_ipt(debuglvl, iface_ptr->device, "INPUT",
                                        &iface_ptr->cnt->input_packets,
                                        &iface_ptr->cnt->input_bytes,
                                        &tmp_ull, &tmp_ull);
            get_iface_stats_from_ipt(debuglvl, iface_ptr->device, "OUTPUT",
                                        &tmp_ull, &tmp_ull,
                                        &iface_ptr->cnt->output_packets,
                                        &iface_ptr->cnt->output_bytes);
            get_iface_stats_from_ipt(debuglvl, iface_ptr->device, "FORWARD",
                                        &iface_ptr->cnt->forwardin_packets,
                                        &iface_ptr->cnt->forwardin_bytes,
                                        &iface_ptr->cnt->forwardout_packets,
                                        &iface_ptr->cnt->forwardout_bytes);

            if(debuglvl >= HIGH)
            {
                (void)vrprint.debug(__FUNC__, "iface_ptr->cnt->input_packets: %llu, iface_ptr->cnt->input_bytes: %llu.",
                                    iface_ptr->cnt->input_packets,
                                    iface_ptr->cnt->input_bytes);
                (void)vrprint.debug(__FUNC__, "iface_ptr->cnt->output_packets: %llu, iface_ptr->cnt->output_bytes: %llu.",
                                    iface_ptr->cnt->output_packets,
                                    iface_ptr->cnt->output_bytes);
            }

            /* assemble the chain-name */
            snprintf(acc_chain, sizeof(acc_chain), "ACC-%s", iface_ptr->device);

            if(debuglvl >= HIGH)
                (void)vrprint.debug(__FUNC__, "acc_chain '%s'.", acc_chain);

            /* get the accounting chains numbers */
            get_iface_stats_from_ipt(debuglvl, iface_ptr->device, acc_chain,
                                        &iface_ptr->cnt->acc_in_packets,
                                        &iface_ptr->cnt->acc_in_bytes,
                                        &iface_ptr->cnt->acc_out_packets,
                                        &iface_ptr->cnt->acc_out_bytes);

            if(debuglvl >= HIGH)
            {
                (void)vrprint.debug(__FUNC__, "iface_ptr->cnt->acc_in_bytes: %llu, iface_ptr->cnt->acc_out_bytes: %llu.",
                                    iface_ptr->cnt->acc_in_bytes,
                                    iface_ptr->cnt->acc_out_bytes);
            }
        }
    }

    return(0);
}


static int
ruleset_clear_interface_counters(const int debuglvl, Interfaces *interfaces)
{
    d_list_node             *d_node = NULL;
    struct InterfaceData_   *iface_ptr = NULL;

    /* safety */
    if(!interfaces)
    {
        (void)vrprint.error(-1, "Internal Error", "parameter problem (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /* loop through the interfaces */
    for(d_node = interfaces->list.top; d_node; d_node = d_node->next)
    {
        if(!(iface_ptr = d_node->data))
        {
            (void)vrprint.error(-1, "Internal Error", "NULL pointer (in: %s:%d).", __FUNC__, __LINE__);
            return(-1);
        }

        if(iface_ptr->cnt != NULL)
        {
            free(iface_ptr->cnt);
            iface_ptr->cnt = NULL;
        }
    }

    return(0);
}


static int
ruleset_store_failed_set(const int debuglvl, const char *file)
{
    char    failed_ruleset_path[32] = "";
    int     result = 0;
    size_t  size = 0;
    
    if(file == NULL)
    {
        (void)vrprint.error(-1, "Internal Error", "parameter problem "
            "(in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }
    
    size = strlcpy(failed_ruleset_path, file, sizeof(failed_ruleset_path));
    if(size >= sizeof(failed_ruleset_path))
    {
        (void)vrprint.error(-1, "Internal Error", "could not create "
            "failed rulset path (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }
    size = strlcat(failed_ruleset_path, ".failed", sizeof(failed_ruleset_path));
    if(size >= sizeof(failed_ruleset_path))
    {
        (void)vrprint.error(-1, "Internal Error", "could not create "
            "failed rulset path (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    result = rename(file, failed_ruleset_path);
    if(result == -1)
    {
        (void)vrprint.error(-1, "Error", "renaming '%s' to '%s' "
            "failed: %s (in: %s:%d).", file, failed_ruleset_path,
            strerror(errno), __FUNC__, __LINE__);
        return(-1);
    }

    return(0);
}


static int
ruleset_log_resultfile(const int debuglvl, char *path)
{
    char    line[256] = "";
    FILE    *fp = NULL;

    /* safety */
    if(path == NULL)
    {
        (void)vrprint.error(-1, "Internal Error", "parameter problem (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /* open the file */
    fp = fopen(path, "r");
    if(fp == NULL)
    {
        (void)vrprint.error(-1, "Error", "opening resultfile '%s' failed: %s (in: %s:%d).",path, strerror(errno), __FUNC__, __LINE__);
        return(-1);
    }

    while(fgets(line, (int)sizeof(line), fp) != NULL)
    {
        if(line[strlen(line)-1] == '\n')
            line[strlen(line)-1] = '\0';

        (void)vrprint.error(-1, "Error", "loading ruleset result: '%s'.", line);
    }

    if(fclose(fp) == -1)
    {
        (void)vrprint.error(-1, "Error", "closing resultfile '%s' failed: %s (in: %s:%d).",path, strerror(errno), __FUNC__, __LINE__);
        return(-1);
    }

    return(0);
}


/*  load_ruleset


    Returncodes:
         0: ok
        -1: error
*/
int
load_ruleset(   const int debuglvl,
                Rules *rules,
                Zones *zones,
                Interfaces *interfaces,
                Services *services,
                BlockList *blocklist,
                IptCap *iptcap,
                struct vuurmuur_config *cnf)
{
    RuleSet ruleset;
    char    cur_ruleset_path[] = "/tmp/vuurmuur-XXXXXX";
    char    cur_result_path[] = "/tmp/vuurmuur-load-result-XXXXXX";
    char    cur_shape_path[] = "/tmp/vuurmuur-shape-XXXXXX";
    int     ruleset_fd = 0,
            result_fd = 0,
            shape_fd = 0;

    /* safety */
    if(!rules || !zones || !interfaces || !services || !iptcap || !cnf || !blocklist)
    {
        (void)vrprint.error(-1, "Internal Error", "parameter problem (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /* setup the ruleset */
    if(ruleset_setup(debuglvl, &ruleset) != 0)
    {
        (void)vrprint.error(-1, "Internal Error", "setting up ruleset failed (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /* store counters */
    if(ruleset_save_interface_counters(debuglvl, interfaces) < 0)
    {
        (void)vrprint.error(-1, "Error", "saving interface counters failed (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /* create the ruleset */
    if(ruleset_create_ruleset(debuglvl, &ruleset, rules, zones, interfaces,
                    services, blocklist, iptcap, cnf) < 0)
    {
        (void)vrprint.error(-1, "Error", "creating ruleset failed "
                "(in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /* clear the counters again */
    if(ruleset_clear_interface_counters(debuglvl, interfaces) < 0)
    {
        (void)vrprint.error(-1, "Error", "clearing interface counters failed (in: %s:%d).", __FUNC__, __LINE__);
        return(-1);
    }

    /* create the tempfile */
    ruleset_fd = create_tempfile(debuglvl, cur_ruleset_path);
    if(ruleset_fd == -1)
    {
        (void)vrprint.error(-1, "Error", "creating rulesetfile failed (in: %s:%d).", __FUNC__, __LINE__);

        ruleset_cleanup(debuglvl, &ruleset);
        return(-1);
    }

    /* create the tempfile */
    result_fd = create_tempfile(debuglvl, cur_result_path);
    if(result_fd == -1)
    {
        (void)vrprint.error(-1, "Error", "creating resultfile failed (in: %s:%d).",
                                    __FUNC__, __LINE__);

        ruleset_cleanup(debuglvl, &ruleset);
        return(-1);
    }

    /* create the tempfile */
    shape_fd = create_tempfile(debuglvl, cur_shape_path);
    if(shape_fd == -1)
    {
        (void)vrprint.error(-1, "Error", "creating shape script file failed (in: %s:%d).",
                                    __FUNC__, __LINE__);

        ruleset_cleanup(debuglvl, &ruleset);
        return(-1);
    }

    /* get the custom chains we have to create */
    if(rules_get_custom_chains(debuglvl, rules) < 0)
    {
        (void)vrprint.error(-1, "Internal Error", "rules_get_chains() failed (in: %s:%d).",
                                    __FUNC__, __LINE__);
        return(-1);
    }
    /* now create the currentrulesetfile */
    if(ruleset_fill_file(debuglvl, rules, &ruleset, iptcap, ruleset_fd, cnf) < 0)
    {
        (void)vrprint.error(-1, "Error", "filling rulesetfile failed (in: %s:%d).",
                                    __FUNC__, __LINE__);

        ruleset_cleanup(debuglvl, &ruleset);
        (void)ruleset_store_failed_set(debuglvl, cur_ruleset_path);
        return(-1);
    }
    /* cleanup */
    d_list_cleanup(debuglvl, &rules->custom_chain_list);
    
    /* now create the shape file */
    if(ruleset_fill_shaping_file(debuglvl, &ruleset, shape_fd) < 0)
    {
        (void)vrprint.error(-1, "Error", "filling rulesetfile failed (in: %s:%d).",
                                    __FUNC__, __LINE__);

        ruleset_cleanup(debuglvl, &ruleset);
        (void)ruleset_store_failed_set(debuglvl, cur_ruleset_path);
        return(-1);
    }

    if(debuglvl >= HIGH)
    {
        (void)vrprint.debug(__FUNC__, "sleeping so you can look into the tmpfile.");
        sleep(15);
    }

    /* load the shaping rules */
    if(ruleset_load_shape_ruleset(debuglvl, cur_shape_path, cur_result_path, cnf) != 0)
    {
        /* oops, something went wrong */
        (void)vrprint.error(-1, "Error", "shape rulesetfile will be stored as '%s.failed' (in: %s:%d).",
                                    cur_shape_path, __FUNC__, __LINE__);
        (void)ruleset_store_failed_set(debuglvl, cur_shape_path);
        (void)ruleset_log_resultfile(debuglvl, cur_result_path);
        ruleset_cleanup(debuglvl, &ruleset);
        return(-1);
    }
    /* now load the iptables ruleset */
    if(ruleset_load_ruleset(debuglvl, cur_ruleset_path, cur_result_path, cnf) != 0)
    {
        /* oops, something went wrong */
        (void)vrprint.error(-1, "Error", "rulesetfile will be stored as '%s.failed' (in: %s:%d).",
                                    cur_ruleset_path, __FUNC__, __LINE__);
        (void)ruleset_store_failed_set(debuglvl, cur_ruleset_path);
        (void)ruleset_log_resultfile(debuglvl, cur_result_path);
        ruleset_cleanup(debuglvl, &ruleset);
        return(-1);
    }

    if(cmdline.keep_file == FALSE)
    {
        /* remove the rules tempfile */
        if(unlink(cur_ruleset_path) == -1)
        {
            (void)vrprint.error(-1, "Error", "removing tempfile "
                    "failed: %s (in: %s:%d).",
                    strerror(errno), __FUNC__, __LINE__);

            ruleset_cleanup(debuglvl, &ruleset);
            return(-1);
        }

        /* remove the result tempfile */
        if(unlink(cur_result_path) == -1)
        {
            (void)vrprint.error(-1, "Error", "removing tempfile "
                    "failed: %s (in: %s:%d).",
                    strerror(errno), __FUNC__, __LINE__);

            ruleset_cleanup(debuglvl, &ruleset);
            return(-1);
        }

        /* remove the shape tempfile */
        if(unlink(cur_shape_path) == -1)
        {
            (void)vrprint.error(-1, "Error", "removing tempfile "
                    "failed: %s (in: %s:%d).",
                    strerror(errno), __FUNC__, __LINE__);

            ruleset_cleanup(debuglvl, &ruleset);
            return(-1);
        }
    }

    /* finaly clean up the mess */
    ruleset_cleanup(debuglvl, &ruleset);

    (void)vrprint.info("Info", "ruleset loading completed successfully.");
    return(0);
}
