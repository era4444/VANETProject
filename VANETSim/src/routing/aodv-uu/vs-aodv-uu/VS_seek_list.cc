/*****************************************************************************
 *
 * Copyright (C) 2001 Uppsala University and Ericsson AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Erik Nordstr�m, <erik.nordstrom@it.uu.se>
 *
 *
 *****************************************************************************/
#define NS_PORT
#define OMNETPP

#include <stdlib.h>

#ifdef NS_PORT
#ifndef OMNETPP
#include "ns/vs_aodv-uu.h"
#else
#include "../vs_aodv_uu_omnet.h"
#endif
#include "vs_list.h"
#else
#include "vs_seek_list.h"
#include "vs_timer_queue_aodv.h"
#include "vs_aodv_timeout.h"
#include "vs_defs_aodv.h"
#include "vs_params.h"
#include "vs_debug_aodv.h"
#include "vs_list.h"
#endif

#ifndef NS_PORT
/* The seek vs_list is a linked vs_list of destinations we are seeking
   (with RREQ's). */

static vs_list(seekhead);

#ifdef vs_seek_list_DEBUG
void vs_seek_list_print();
#endif
#endif              /* NS_PORT */
#ifndef vs_aodv_USE_STL
vs_seek_list_t *NS_CLASS vs_seek_list_insert(struct in_addr dest_addr,
                                       u_int32_t dest_seqno,
                                       int ttl, u_int8_t flags,
                                       struct ip_data *ipd)
{
    vs_seek_list_t *entry;

    if ((entry = (vs_seek_list_t *) malloc(sizeof(vs_seek_list_t))) == NULL)
    {
        fprintf(stderr, "Failed malloc\n");
        exit(-1);
    }

    entry->dest_addr = dest_addr;
    entry->dest_seqno = dest_seqno;
    entry->flags = flags;
    entry->reqs = 0;
    entry->ttl = ttl;
    entry->ipd = ipd;

    timer_init(&entry->seek_timer, &NS_CLASS route_discovery_timeout, entry);

    vs_list_add(&seekhead, &entry->l);
#ifdef vs_seek_list_DEBUG
    vs_seek_list_print();
#endif
    return entry;
}

int NS_CLASS vs_seek_list_remove(vs_seek_list_t * entry)
{
    if (!entry)
        return 0;

    vs_list_detach(&entry->l);

    /* Make sure any timers are removed */
    timer_remove(&entry->seek_timer);

    if (entry->ipd)
        free(entry->ipd);

    free(entry);
    return 1;
}

vs_seek_list_t *NS_CLASS vs_seek_list_find(struct in_addr dest_addr)
{
    vs_list_t *pos;

    vs_list_foreach(pos, &seekhead)
    {
        vs_seek_list_t *entry = (vs_seek_list_t *) pos;

        if (entry->dest_addr.s_addr == dest_addr.s_addr)
            return entry;
    }
    return NULL;
}

#ifdef vs_seek_list_DEBUG
void NS_CLASS vs_seek_list_print()
{
    vs_list_t *pos;

    vs_list_foreach(pos, &seekhead)
    {
        vs_seek_list_t *entry = (vs_seek_list_t *) pos;
        printf("%s %u %d %d\n", ip_to_str(entry->dest_addr),
               entry->dest_seqno, entry->reqs, entry->ttl);
    }
}
#endif
#else
vs_seek_list_t *NS_CLASS vs_seek_list_insert(struct in_addr dest_addr,
                                       u_int32_t dest_seqno,
                                       int ttl, u_int8_t flags,
                                       struct ip_data *ipd)
{
    vs_seek_list_t *entry;

    entry = new vs_seek_list_t;
    if (entry == NULL)
    {
        fprintf(stderr, "Failed malloc\n");
        exit(-1);
    }

    entry->dest_addr = dest_addr;
    entry->dest_seqno = dest_seqno;
    entry->flags = flags;
    entry->reqs = 0;
    entry->ttl = ttl;
    entry->ipd = ipd;

    timer_init(&entry->seek_timer, &NS_CLASS route_discovery_timeout, entry);
    seekhead.insert(std::make_pair(dest_addr.s_addr,entry));

#ifdef vs_seek_list_DEBUG
    vs_seek_list_print();
#endif
    return entry;
}

int NS_CLASS vs_seek_list_remove(vs_seek_list_t * entry)
{
    if (!entry)
        return 0;

    for (SeekHead::iterator it =seekhead.begin();it != seekhead.end(); it++)
    {
        if (it->second == entry)
        {
            seekhead.erase(it);
            break;
        }
    }

    /* Make sure any timers are removed */
    timer_remove(&entry->seek_timer);

    if (entry->ipd)
        free(entry->ipd);

    delete entry;
    return 1;
}

vs_seek_list_t *NS_CLASS vs_seek_list_find(struct in_addr dest_addr)
{
    SeekHead::iterator it =seekhead.find(dest_addr.s_addr);
    if (it != seekhead.end())
        return it->second;
    return NULL;
}

#ifdef vs_seek_list_DEBUG
void NS_CLASS vs_seek_list_print()
{
    for (SeekHead::iterator it =seekhead.begin();it != seekhead.end(); it++)
    {
        vs_seek_list_t *entry = it->second;
        printf("%s %u %d %d\n", ip_to_str(entry->dest_addr),
                      entry->dest_seqno, entry->reqs, entry->ttl);    }
}
#endif
#endif
