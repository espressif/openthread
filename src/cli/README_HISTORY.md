# OpenThread CLI - History Tracker

History Tracker module records history of different events (e.g., RX and TX IPv6 messages or network info changes, etc.) as the Thread network operates. All tracked entries are timestamped.

All commands under `history` require `OPENTHREAD_CONFIG_HISTORY_TRACKER_ENABLE` feature to be enabled.

The number of entries recorded for each history list is configurable through a set of OpenThread config options, e.g. `OPENTHREAD_CONFIG_HISTORY_TRACKER_NET_INFO_LIST_SIZE` specifies the number of entries in Network Info history list. The History Tracker will keep the most recent entries overwriting oldest one when the list gets full.

## Command List

Usage : `history [command] ...`

- [help](#help)
- [dnssrpaddr](#dnssrpaddr)
- [ipaddr](#ipaddr)
- [ipmaddr](#ipmaddr)
- [neighbor](#neighbor)
- [netinfo](#netinfo)
- [prefix](#prefix)
- [route](#route)
- [router](#router)
- [rx](#rx)
- [rxtx](#rxtx)
- [tx](#tx)

## Timestamp Format

Recorded entries are timestamped. When the history list is printed, the timestamps are shown relative the time the command was issues (i.e., when the list was printed) indicating how long ago the entry was recorded.

```bash
> history netinfo
| Age                  | Role     | Mode | RLOC16 | Partition ID |
+----------------------+----------+------+--------+--------------+
|         02:31:50.628 | leader   | rdn  | 0x2000 |    151029327 |
|         02:31:53.262 | detached | rdn  | 0xfffe |            0 |
|         02:31:54.663 | detached | rdn  | 0x2000 |            0 |
Done
```

For example `02:31:50.628` indicates the event was recorded "2 hours, 31 minutes, 50 seconds, and 628 milliseconds ago". Number of days is added for events that are older than 24 hours, e.g., `1 day 11:25:31.179`, or `31 days 03:00:23.931`.

Timestamps use millisecond accuracy and are tacked up to 49 days. If the event is older than 49 days, the entry is still tracked in the list but the timestamp is shown as `more than 49 days`.

## Command Details

### help

Usage: `history help`

Print SRP client help menu.

```bash
> history help
help
ipaddr
ipmaddr
neighbor
netinfo
prefix
route
router
rx
rxtx
tx
Done
>
```

### dnssrpaddr

Usage `history dnssrpaddr [list] [<num-entries>]`

Print the network data DNS/SRP address history. Each entry provides:

- Event: Possible values are `Added` or `Removed`.
- Address of DNS/SRP server.
- Address type: `uni-local` unicast address local (address in server data) or `uni-infra` infrastructure server (addr in service data), or `anycast`.
- Port: Server port number, only applicable when address type is unicast.
- Sequence Number: Anycast sequence number, only applicable when address type is anycast.
- Version number.
- RLOC16 of the BR adding/removing this entry in Network Data.

```bash
> history dnssrpaddr
| Age                  | Event   | Address                                 | Type      |Port/Seq| Ver | RLOC16 |
+----------------------+---------+-----------------------------------------+-----------+--------+-----+--------+
|         00:00:07.150 | Added   | fd4f:59ae:348a:aa48:74a4:6de9:7a30:5dfb | uni-local |   1234 |   0 | 0x0000 |
|         00:00:09.351 | Removed | fd00:0:0:0:0:0:0:1234                   | uni-infra |  51525 |   1 | 0x0000 |
|         00:00:28.479 | Added   | fd00:0:0:0:0:0:0:1234                   | uni-infra |  51525 |   1 | 0x0000 |
|         00:00:30.133 | Removed | fd4f:59ae:348a:aa48:0:ff:fe00:fc10      |   anycast |      1 |   2 | 0x0000 |
|         00:01:02.609 | Added   | fd4f:59ae:348a:aa48:0:ff:fe00:fc10      |   anycast |      1 |   2 | 0x0000 |
|         00:01:03.574 | Removed | fd4f:59ae:348a:aa48:74a4:6de9:7a30:5dfb | uni-local |  50152 |   2 | 0x0000 |
|         00:01:33.631 | Added   | fd4f:59ae:348a:aa48:74a4:6de9:7a30:5dfb | uni-local |  50152 |   2 | 0x0000 |
Done
```

Print the history as a list.

```bash
> history dnssrpaddr list
00:00:19.646 -> event:Added addr:fd4f:59ae:348a:aa48:74a4:6de9:7a30:5dfb type:uni-local port:1234 ver:0 rloc16:0x0000
00:00:21.847 -> event:Removed addr:fd00:0:0:0:0:0:0:1234 type:uni-infra port:51525 ver:1 rloc16:0x0000
00:00:40.975 -> event:Added addr:fd00:0:0:0:0:0:0:1234 type:uni-infra port:51525 ver:1 rloc16:0x0000
00:00:42.629 -> event:Removed addr:fd4f:59ae:348a:aa48:0:ff:fe00:fc10 type:anycast seqno:1 ver:2 rloc16:0x0000
00:01:15.105 -> event:Added addr:fd4f:59ae:348a:aa48:0:ff:fe00:fc10 type:anycast seqno:1 ver:2 rloc16:0x0000
00:01:16.070 -> event:Removed addr:fd4f:59ae:348a:aa48:74a4:6de9:7a30:5dfb type:uni-local port:50152 ver:2 rloc16:0x0000
00:01:46.127 -> event:Added addr:fd4f:59ae:348a:aa48:74a4:6de9:7a30:5dfb type:uni-local port:50152 ver:2 rloc16:0x0000
Done
```

### ipaddr

Usage `history ipaddr [list] [<num-entries>]`

Print the unicast IPv6 address history. Each entry provides:

- Event: Added or Removed.
- Address: Unicast address along with its prefix length (in bits).
- Origin: thread, slaac, dhcp6, or manual.
- Address Scope.
- Flags: Preferred, Valid, and RLOC (whether the address is RLOC).

Print the unicast IPv6 address history as table.

```bash
> history ipaddr
| Age                  | Event   | Address / PrefixLength                      | Origin |Scope| P | V | R |
+----------------------+---------+---------------------------------------------+--------+-----+---+---+---+
|         00:00:04.991 | Removed | 2001:dead:beef:cafe:c4cb:caba:8d55:e30b/64  | slaac  |  14 | Y | Y | N |
|         00:00:44.647 | Added   | 2001:dead:beef:cafe:c4cb:caba:8d55:e30b/64  | slaac  |  14 | Y | Y | N |
|         00:01:07.199 | Added   | fd00:0:0:0:0:0:0:1/64                       | manual |  14 | Y | Y | N |
|         00:02:17.885 | Added   | fdde:ad00:beef:0:0:ff:fe00:fc00/64          | thread |   3 | N | Y | N |
|         00:02:17.885 | Added   | fdde:ad00:beef:0:0:ff:fe00:5400/64          | thread |   3 | N | Y | Y |
|         00:02:20.107 | Removed | fdde:ad00:beef:0:0:ff:fe00:5400/64          | thread |   3 | N | Y | Y |
|         00:02:21.575 | Added   | fdde:ad00:beef:0:0:ff:fe00:5400/64          | thread |   3 | N | Y | Y |
|         00:02:21.575 | Added   | fdde:ad00:beef:0:ecea:c4fc:ad96:4655/64     | thread |   3 | N | Y | N |
|         00:02:23.904 | Added   | fe80:0:0:0:3c12:a4d2:fbe0:31ad/64           | thread |   2 | Y | Y | N |
Done
```

Print the unicast IPv6 address history as a list (the last 5 entries).

```bash
> history ipaddr list 5
00:00:20.327 -> event:Removed address:2001:dead:beef:cafe:c4cb:caba:8d55:e30b prefixlen:64 origin:slaac scope:14 preferred:yes valid:yes rloc:no
00:00:59.983 -> event:Added address:2001:dead:beef:cafe:c4cb:caba:8d55:e30b prefixlen:64 origin:slaac scope:14 preferred:yes valid:yes rloc:no
00:01:22.535 -> event:Added address:fd00:0:0:0:0:0:0:1 prefixlen:64 origin:manual scope:14 preferred:yes valid:yes rloc:no
00:02:33.221 -> event:Added address:fdde:ad00:beef:0:0:ff:fe00:fc00 prefixlen:64 origin:thread scope:3 preferred:no valid:yes rloc:no
00:02:33.221 -> event:Added address:fdde:ad00:beef:0:0:ff:fe00:5400 prefixlen:64 origin:thread scope:3 preferred:no valid:yes rloc:yes
Done
```

### ipmaddr

Usage `history ipmaddr [list] [<num-entries>]`

Print the multicast IPv6 address history. Each entry provides:

- Event: Subscribed or Unsubscribed.
- Address: Multicast address.
- Origin: Thread, or Manual.

Print the multicast IPv6 address history as table.

```bash
> history ipmaddr
| Age                  | Event        | Multicast Address                       | Origin |
+----------------------+--------------+-----------------------------------------+--------+
|         00:00:08.592 | Unsubscribed | ff05:0:0:0:0:0:0:1                      | Manual |
|         00:01:25.353 | Subscribed   | ff05:0:0:0:0:0:0:1                      | Manual |
|         00:01:54.953 | Subscribed   | ff03:0:0:0:0:0:0:2                      | Thread |
|         00:01:54.953 | Subscribed   | ff02:0:0:0:0:0:0:2                      | Thread |
|         00:01:59.329 | Subscribed   | ff33:40:fdde:ad00:beef:0:0:1            | Thread |
|         00:01:59.329 | Subscribed   | ff32:40:fdde:ad00:beef:0:0:1            | Thread |
|         00:02:01.129 | Subscribed   | ff03:0:0:0:0:0:0:fc                     | Thread |
|         00:02:01.129 | Subscribed   | ff03:0:0:0:0:0:0:1                      | Thread |
|         00:02:01.129 | Subscribed   | ff02:0:0:0:0:0:0:1                      | Thread |
Done
```

Print the multicast IPv6 address history as a list.

```bash
> history ipmaddr list
00:00:25.447 -> event:Unsubscribed address:ff05:0:0:0:0:0:0:1 origin:Manual
00:01:42.208 -> event:Subscribed address:ff05:0:0:0:0:0:0:1 origin:Manual
00:02:11.808 -> event:Subscribed address:ff03:0:0:0:0:0:0:2 origin:Thread
00:02:11.808 -> event:Subscribed address:ff02:0:0:0:0:0:0:2 origin:Thread
00:02:16.184 -> event:Subscribed address:ff33:40:fdde:ad00:beef:0:0:1 origin:Thread
00:02:16.184 -> event:Subscribed address:ff32:40:fdde:ad00:beef:0:0:1 origin:Thread
00:02:17.984 -> event:Subscribed address:ff03:0:0:0:0:0:0:fc origin:Thread
00:02:17.984 -> event:Subscribed address:ff03:0:0:0:0:0:0:1 origin:Thread
00:02:17.984 -> event:Subscribed address:ff02:0:0:0:0:0:0:1 origin:Thread
Done
```

### neighbor

Usage `history neighbor [list] [<num-entries>]`

Print the neighbor table history. Each entry provides:

- Type: Child or Router
- Event: Added, Removed, Changed (e.g., mode change).
- Extended Address
- RLOC16
- MLE Link Mode
- Average RSS (in dBm) of received frames from neighbor at the time the entry was recorded

Print the neighbor history as a table.

```bash
> history neighbor
| Age                  | Type   | Event     | Extended Address | RLOC16 | Mode | Ave RSS |
+----------------------+--------+-----------+------------------+--------+------+---------+
|         00:00:29.233 | Child  | Added     | ae5105292f0b9169 | 0x8404 | -    |     -20 |
|         00:01:38.368 | Child  | Removed   | ae5105292f0b9169 | 0x8401 | -    |     -20 |
|         00:04:27.181 | Child  | Changed   | ae5105292f0b9169 | 0x8401 | -    |     -20 |
|         00:04:51.236 | Router | Added     | 865c7ca38a5fa960 | 0x9400 | rdn  |     -20 |
|         00:04:51.587 | Child  | Removed   | 865c7ca38a5fa960 | 0x8402 | rdn  |     -20 |
|         00:05:22.764 | Child  | Changed   | ae5105292f0b9169 | 0x8401 | rn   |     -20 |
|         00:06:40.764 | Child  | Added     | 4ec99efc874a1841 | 0x8403 | r    |     -20 |
|         00:06:44.060 | Child  | Added     | 865c7ca38a5fa960 | 0x8402 | rdn  |     -20 |
|         00:06:49.515 | Child  | Added     | ae5105292f0b9169 | 0x8401 | -    |     -20 |
Done
```

Print the neighbor history as a list.

```bash

> history neighbor list
00:00:34.753 -> type:Child event:Added extaddr:ae5105292f0b9169 rloc16:0x8404 mode:- rss:-20
00:01:43.888 -> type:Child event:Removed extaddr:ae5105292f0b9169 rloc16:0x8401 mode:- rss:-20
00:04:32.701 -> type:Child event:Changed extaddr:ae5105292f0b9169 rloc16:0x8401 mode:- rss:-20
00:04:56.756 -> type:Router event:Added extaddr:865c7ca38a5fa960 rloc16:0x9400 mode:rdn rss:-20
00:04:57.107 -> type:Child event:Removed extaddr:865c7ca38a5fa960 rloc16:0x8402 mode:rdn rss:-20
00:05:28.284 -> type:Child event:Changed extaddr:ae5105292f0b9169 rloc16:0x8401 mode:rn rss:-20
00:06:46.284 -> type:Child event:Added extaddr:4ec99efc874a1841 rloc16:0x8403 mode:r rss:-20
00:06:49.580 -> type:Child event:Added extaddr:865c7ca38a5fa960 rloc16:0x8402 mode:rdn rss:-20
00:06:55.035 -> type:Child event:Added extaddr:ae5105292f0b9169 rloc16:0x8401 mode:- rss:-20
Done
```

### netinfo

Usage `history netinfo [list] [<num-entries>]`

Print the Network Info history. Each Network Info provides:

- Device Role
- MLE Link Mode
- RLOC16
- Partition ID

Print the Network Info history as a table.

```bash
> history netinfo
| Age                  | Role     | Mode | RLOC16 | Partition ID |
+----------------------+----------+------+--------+--------------+
|         00:00:10.069 | router   | rdn  | 0x6000 |    151029327 |
|         00:02:09.337 | child    | rdn  | 0x2001 |    151029327 |
|         00:02:09.338 | child    | rdn  | 0x2001 |    151029327 |
|         00:07:40.806 | child    | -    | 0x2001 |    151029327 |
|         00:07:42.297 | detached | -    | 0x6000 |            0 |
|         00:07:42.968 | disabled | -    | 0x6000 |            0 |
Done
```

Print the Network Info history as a list.

```bash
> history netinfo list
00:00:59.467 -> role:router mode:rdn rloc16:0x6000 partition-id:151029327
00:02:58.735 -> role:child mode:rdn rloc16:0x2001 partition-id:151029327
00:02:58.736 -> role:child mode:rdn rloc16:0x2001 partition-id:151029327
00:08:30.204 -> role:child mode:- rloc16:0x2001 partition-id:151029327
00:08:31.695 -> role:detached mode:- rloc16:0x6000 partition-id:0
00:08:32.366 -> role:disabled mode:- rloc16:0x6000 partition-id:0
Done
```

Print only the latest 2 entries.

```bash
> history netinfo 2
| Age                  | Role     | Mode | RLOC16 | Partition ID |
+----------------------+----------+------+--------+--------------+
|         00:02:05.451 | router   | rdn  | 0x6000 |    151029327 |
|         00:04:04.719 | child    | rdn  | 0x2001 |    151029327 |
Done
```

### prefix

Usage `history prefix [list] [<num-entries>]`

Print the Network Data on mesh prefix history. Each item provides:

- Event (`Added` or `Removed`)
- Prefix
- Flags
- Preference (`high`, `med`, `low`)
- RLOC16

The flags are as follows:

- `p`: Preferred flag
- `a`: Stateless IPv6 Address Autoconfiguration flag
- `d`: DHCPv6 IPv6 Address Configuration flag
- `c`: DHCPv6 Other Configuration flag
- `r`: Default Route flag
- `o`: On Mesh flag
- `s`: Stable flag
- `n`: Nd Dns flag
- `D`: Domain Prefix flag

Print the history as a table.

```bash
> history prefix
| Age                  | Event   | Prefix                                      | Flags     | Pref | RLOC16 |
+----------------------+---------+---------------------------------------------+-----------+------+--------+
|         00:00:10.663 | Added   | fd00:1111:2222:3333::/64                    | paro      | med  | 0x5400 |
|         00:01:02.054 | Removed | fd00:dead:beef:1::/64                       | paros     | high | 0x5400 |
|         00:01:21.136 | Added   | fd00:abba:cddd:0::/64                       | paos      | med  | 0x5400 |
|         00:01:45.144 | Added   | fd00:dead:beef:1::/64                       | paros     | high | 0x3c00 |
|         00:01:50.944 | Added   | fd00:dead:beef:1::/64                       | paros     | high | 0x5400 |
|         00:01:59.887 | Added   | fd00:dead:beef:1::/64                       | paros     | med  | 0x8800 |
Done
```

Print the history as a list.

```bash
> history prefix list
00:04:12.487 -> event:Added prefix:fd00:1111:2222:3333::/64 flags:paro pref:med rloc16:0x5400
00:05:03.878 -> event:Removed prefix:fd00:dead:beef:1::/64 flags:paros pref:high rloc16:0x5400
00:05:22.960 -> event:Added prefix:fd00:abba:cddd:0::/64 flags:paos pref:med rloc16:0x5400
00:05:46.968 -> event:Added prefix:fd00:dead:beef:1::/64 flags:paros pref:high rloc16:0x3c00
00:05:52.768 -> event:Added prefix:fd00:dead:beef:1::/64 flags:paros pref:high rloc16:0x5400
00:06:01.711 -> event:Added prefix:fd00:dead:beef:1::/64 flags:paros pref:med rloc16:0x8800
```

### route

Usage `history route [list] [<num-entries>]`

Print the Network Data external route history. Each item provides:

- Event (`Added` or `Removed`)
- Route
- Flags
- Preference (`high`, `med`, `low`)
- RLOC16

The flags are as follows:

- `s`: Stable flag
- `n`: NAT64 flag

Print the history as a table.

```bash
history route
| Age                  | Event   | Route                                       | Flags     | Pref | RLOC16 |
+----------------------+---------+---------------------------------------------+-----------+------+--------+
|         00:00:05.456 | Removed | fd00:1111:0::/48                            | s         | med  | 0x3c00 |
|         00:00:29.310 | Added   | fd00:1111:0::/48                            | s         | med  | 0x3c00 |
|         00:00:42.822 | Added   | fd00:1111:0::/48                            | s         | med  | 0x5400 |
|         00:01:27.688 | Added   | fd00:aaaa:bbbb:cccc::/64                    | s         | med  | 0x8800 |
Done
```

Print the history as a list (last two entries).

```bash
> history route list 2
00:00:48.704 -> event:Removed route:fd00:1111:0::/48 flags:s pref:med rloc16:0x3c00
00:01:12.558 -> event:Added route:fd00:1111:0::/48 flags:s pref:med rloc16:0x3c00
Done
```

### router

Usage `history router [list] [<num-entries>]`

Print the route table history. Each item provides:

- Event (`Added`, `Removed`, `NextHopChanged`, `CostChanged`)
- Router ID and RLOC16 of router
- Next Hop (Router ID and RLOC16) - `none` if no next hop.
- Path cost (old `->` new) - `inf` to indicate infinite path cost.

Print the history as a table.

```bash
> history router
| Age                  | Event          | ID (RLOC16) | Next Hop    | Path Cost  |
+----------------------+----------------+-------------+-------------+------------+
|         00:00:05.258 | NextHopChanged |  7 (0x1c00) | 34 (0x8800) | inf ->   3 |
|         00:00:08.604 | NextHopChanged | 34 (0x8800) | 34 (0x8800) | inf ->   2 |
|         00:00:08.604 | Added          |  7 (0x1c00) |        none | inf -> inf |
|         00:00:11.931 | Added          | 34 (0x8800) |        none | inf -> inf |
|         00:00:14.948 | Removed        | 59 (0xec00) |        none | inf -> inf |
|         00:00:14.948 | Removed        | 54 (0xd800) |        none | inf -> inf |
|         00:00:14.948 | Removed        | 34 (0x8800) |        none | inf -> inf |
|         00:00:14.948 | Removed        |  7 (0x1c00) |        none | inf -> inf |
|         00:00:54.795 | NextHopChanged | 59 (0xec00) | 34 (0x8800) |   1 ->   5 |
|         00:02:33.735 | NextHopChanged | 54 (0xd800) |        none |  15 -> inf |
|         00:03:10.915 | CostChanged    | 54 (0xd800) | 34 (0x8800) |  13 ->  15 |
|         00:03:45.716 | NextHopChanged | 54 (0xd800) | 34 (0x8800) |  15 ->  13 |
|         00:03:46.188 | CostChanged    | 54 (0xd800) | 59 (0xec00) |  13 ->  15 |
|         00:04:19.124 | CostChanged    | 54 (0xd800) | 59 (0xec00) |  11 ->  13 |
|         00:04:52.008 | CostChanged    | 54 (0xd800) | 59 (0xec00) |   9 ->  11 |
|         00:05:23.176 | CostChanged    | 54 (0xd800) | 59 (0xec00) |   7 ->   9 |
|         00:05:51.081 | CostChanged    | 54 (0xd800) | 59 (0xec00) |   5 ->   7 |
|         00:06:48.721 | CostChanged    | 54 (0xd800) | 59 (0xec00) |   3 ->   5 |
|         00:07:13.792 | NextHopChanged | 54 (0xd800) | 59 (0xec00) |   1 ->   3 |
|         00:09:28.681 | NextHopChanged |  7 (0x1c00) | 34 (0x8800) | inf ->   3 |
|         00:09:31.882 | Added          |  7 (0x1c00) |        none | inf -> inf |
|         00:09:51.240 | NextHopChanged | 54 (0xd800) | 54 (0xd800) | inf ->   1 |
|         00:09:54.204 | Added          | 54 (0xd800) |        none | inf -> inf |
|         00:10:20.645 | NextHopChanged | 34 (0x8800) | 34 (0x8800) | inf ->   2 |
|         00:10:24.242 | NextHopChanged | 59 (0xec00) | 59 (0xec00) | inf ->   1 |
|         00:10:24.242 | Added          | 34 (0x8800) |        none | inf -> inf |
|         00:10:41.900 | NextHopChanged | 59 (0xec00) |        none |   1 -> inf |
|         00:10:42.480 | Added          |  3 (0x0c00) |  3 (0x0c00) | inf -> inf |
|         00:10:43.614 | Added          | 59 (0xec00) | 59 (0xec00) | inf ->   1 |
Done
```

Print the history as a list (last 20 entries).

```bash
> history router list 20
00:00:06.959 -> event:NextHopChanged router:7(0x1c00) nexthop:34(0x8800) old-cost:inf new-cost:3
00:00:10.305 -> event:NextHopChanged router:34(0x8800) nexthop:34(0x8800) old-cost:inf new-cost:2
00:00:10.305 -> event:Added router:7(0x1c00) nexthop:none old-cost:inf new-cost:inf
00:00:13.632 -> event:Added router:34(0x8800) nexthop:none old-cost:inf new-cost:inf
00:00:16.649 -> event:Removed router:59(0xec00) nexthop:none old-cost:inf new-cost:inf
00:00:16.649 -> event:Removed router:54(0xd800) nexthop:none old-cost:inf new-cost:inf
00:00:16.649 -> event:Removed router:34(0x8800) nexthop:none old-cost:inf new-cost:inf
00:00:16.649 -> event:Removed router:7(0x1c00) nexthop:none old-cost:inf new-cost:inf
00:00:56.496 -> event:NextHopChanged router:59(0xec00) nexthop:34(0x8800) old-cost:1 new-cost:5
00:02:35.436 -> event:NextHopChanged router:54(0xd800) nexthop:none old-cost:15 new-cost:inf
00:03:12.616 -> event:CostChanged router:54(0xd800) nexthop:34(0x8800) old-cost:13 new-cost:15
00:03:47.417 -> event:NextHopChanged router:54(0xd800) nexthop:34(0x8800) old-cost:15 new-cost:13
00:03:47.889 -> event:CostChanged router:54(0xd800) nexthop:59(0xec00) old-cost:13 new-cost:15
00:04:20.825 -> event:CostChanged router:54(0xd800) nexthop:59(0xec00) old-cost:11 new-cost:13
00:04:53.709 -> event:CostChanged router:54(0xd800) nexthop:59(0xec00) old-cost:9 new-cost:11
00:05:24.877 -> event:CostChanged router:54(0xd800) nexthop:59(0xec00) old-cost:7 new-cost:9
00:05:52.782 -> event:CostChanged router:54(0xd800) nexthop:59(0xec00) old-cost:5 new-cost:7
00:06:50.422 -> event:CostChanged router:54(0xd800) nexthop:59(0xec00) old-cost:3 new-cost:5
00:07:15.493 -> event:NextHopChanged router:54(0xd800) nexthop:59(0xec00) old-cost:1 new-cost:3
00:09:30.382 -> event:NextHopChanged router:7(0x1c00) nexthop:34(0x8800) old-cost:inf new-cost:3
Done
```

### rx

Usage `history rx [list] [<num-entries>]`

Print the IPv6 message RX history in either table or list format. Entries provide same information and follow same format as in `history rxtx` command.

Print the IPv6 message RX history as a table:

```bash
> history rx
| Age                  | Type             | Len   | Chksum | Sec | Prio | RSS  |Dir | Neighb | Radio |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |    50 | 0xbd26 |  no |  net |  -20 | RX | 0x4800 |  15.4 |
|         00:00:07.640 | src: [fe80:0:0:0:d03d:d3e7:cc5e:7cd7]:19788                                 |
|                      | dst: [ff02:0:0:0:0:0:0:1]:19788                                             |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | HopOpts          |    44 | 0x0000 | yes | norm |  -20 | RX | 0x4800 |  15.4 |
|         00:00:09.263 | src: [fdde:ad00:beef:0:0:ff:fe00:4800]:0                                    |
|                      | dst: [ff03:0:0:0:0:0:0:2]:0                                                 |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |    12 | 0x3f7d | yes |  net |  -20 | RX | 0x4800 |  15.4 |
|         00:00:09.302 | src: [fdde:ad00:beef:0:0:ff:fe00:4800]:61631                                |
|                      | dst: [fdde:ad00:beef:0:0:ff:fe00:4801]:61631                                |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | ICMP6(EchoReqst) |    16 | 0x942c | yes | norm |  -20 | RX | 0x4800 |  15.4 |
|         00:00:09.304 | src: [fdde:ad00:beef:0:ac09:a16b:3204:dc09]:0                               |
|                      | dst: [fdde:ad00:beef:0:dc0e:d6b3:f180:b75b]:0                               |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | HopOpts          |    44 | 0x0000 | yes | norm |  -20 | RX | 0x4800 |  15.4 |
|         00:00:09.304 | src: [fdde:ad00:beef:0:0:ff:fe00:4800]:0                                    |
|                      | dst: [ff03:0:0:0:0:0:0:2]:0                                                 |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |    50 | 0x2e37 |  no |  net |  -20 | RX | 0x4800 |  15.4 |
|         00:00:21.622 | src: [fe80:0:0:0:d03d:d3e7:cc5e:7cd7]:19788                                 |
|                      | dst: [ff02:0:0:0:0:0:0:1]:19788                                             |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |    50 | 0xe177 |  no |  net |  -20 | RX | 0x4800 |  15.4 |
|         00:00:26.640 | src: [fe80:0:0:0:d03d:d3e7:cc5e:7cd7]:19788                                 |
|                      | dst: [ff02:0:0:0:0:0:0:1]:19788                                             |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |   165 | 0x82ee | yes |  net |  -20 | RX | 0x4800 |  15.4 |
|         00:00:30.000 | src: [fe80:0:0:0:d03d:d3e7:cc5e:7cd7]:19788                                 |
|                      | dst: [fe80:0:0:0:a4a5:bbac:a8e:bd07]:19788                                  |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |    93 | 0x52df |  no |  net |  -20 | RX | unknwn |  15.4 |
|         00:00:30.480 | src: [fe80:0:0:0:d03d:d3e7:cc5e:7cd7]:19788                                 |
|                      | dst: [fe80:0:0:0:a4a5:bbac:a8e:bd07]:19788                                  |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |    50 | 0x5ccf |  no |  net |  -20 | RX | unknwn |  15.4 |
|         00:00:30.772 | src: [fe80:0:0:0:d03d:d3e7:cc5e:7cd7]:19788                                 |
|                      | dst: [ff02:0:0:0:0:0:0:1]:19788                                             |
Done

```

Print the latest 5 entries of the IPv6 message RX history as a list:

```bash
> history rx list 4
00:00:13.368
    type:UDP len:50 checksum:0xbd26 sec:no prio:net rss:-20 from:0x4800 radio:15.4
    src:[fe80:0:0:0:d03d:d3e7:cc5e:7cd7]:19788
    dst:[ff02:0:0:0:0:0:0:1]:19788
00:00:14.991
    type:HopOpts len:44 checksum:0x0000 sec:yes prio:norm rss:-20 from:0x4800 radio:15.4
    src:[fdde:ad00:beef:0:0:ff:fe00:4800]:0
    dst:[ff03:0:0:0:0:0:0:2]:0
00:00:15.030
    type:UDP len:12 checksum:0x3f7d sec:yes prio:net rss:-20 from:0x4800 radio:15.4
    src:[fdde:ad00:beef:0:0:ff:fe00:4800]:61631
    dst:[fdde:ad00:beef:0:0:ff:fe00:4801]:61631
00:00:15.032
    type:ICMP6(EchoReqst) len:16 checksum:0x942c sec:yes prio:norm rss:-20 from:0x4800 radio:15.4
    src:[fdde:ad00:beef:0:ac09:a16b:3204:dc09]:0
    dst:[fdde:ad00:beef:0:dc0e:d6b3:f180:b75b]:0
Done
```

### rxtx

Usage `history rxtx [list] [<num-entries>]`

Print the combined IPv6 message RX and TX history in either table or list format. Each entry provides:

- IPv6 message type: UDP, TCP, ICMP6 (and its subtype), etc.
- IPv6 payload length (excludes the IPv6 header).
- Source IPv6 address and port number.
- Destination IPv6 address and port number (port number is valid for UDP/TCP, it is zero otherwise).
- Whether or not link-layer security was used.
- Message priority: low, norm, high, net (for Thread control messages).
- Message checksum (valid for UDP, TCP, or ICMP6 message)
- RSS: Received Signal Strength (in dBm) - averaged over all received fragment frames that formed the message. For TX history `NA` (not applicable) is used.
- Whether the message was sent or received (`TX` or `RX`). A failed transmission (e.g., if tx was aborted or no ack from peer for any of the message fragments) is indicated with `TX-F` in the table format or `tx-success:no` in the list format.
- Short address (RLOC16) of neighbor to/from which the message was sent/received. If the frame is broadcast, it is shown as `bcast` in table format or `0xffff` in the list format. If the short address of neighbor is not available, it is shown as `unknwn` in the table format or `0xfffe` in the list format.
- Radio link on which the message was sent/received (useful when `OPENTHREAD_CONFIG_MULTI_RADIO` is enabled). Can be `15.4`, `trel`, or `all` (if sent on all radio links).

Print the IPv6 message RX and TX history as a table:

```bash
> history rxtx
| Age                  | Type             | Len   | Chksum | Sec | Prio | RSS  |Dir | Neighb | Radio |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | HopOpts          |    44 | 0x0000 | yes | norm |  -20 | RX | 0x0800 |  15.4 |
|         00:00:09.267 | src: [fdde:ad00:beef:0:0:ff:fe00:800]:0                                     |
|                      | dst: [ff03:0:0:0:0:0:0:2]:0                                                 |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |    12 | 0x6c6b | yes |  net |  -20 | RX | 0x0800 |  15.4 |
|         00:00:09.290 | src: [fdde:ad00:beef:0:0:ff:fe00:800]:61631                                 |
|                      | dst: [fdde:ad00:beef:0:0:ff:fe00:801]:61631                                 |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | ICMP6(EchoReqst) |    16 | 0xc6a2 | yes | norm |  -20 | RX | 0x0800 |  15.4 |
|         00:00:09.292 | src: [fdde:ad00:beef:0:efe8:4910:cf95:dee9]:0                               |
|                      | dst: [fdde:ad00:beef:0:af4c:3644:882a:3698]:0                               |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | ICMP6(EchoReply) |    16 | 0xc5a2 | yes | norm |  NA  | TX | 0x0800 |  15.4 |
|         00:00:09.292 | src: [fdde:ad00:beef:0:af4c:3644:882a:3698]:0                               |
|                      | dst: [fdde:ad00:beef:0:efe8:4910:cf95:dee9]:0                               |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |    50 | 0xaa0d | yes |  net |  NA  | TX | 0x0800 |  15.4 |
|         00:00:09.294 | src: [fdde:ad00:beef:0:0:ff:fe00:801]:61631                                 |
|                      | dst: [fdde:ad00:beef:0:0:ff:fe00:800]:61631                                 |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | HopOpts          |    44 | 0x0000 | yes | norm |  -20 | RX | 0x0800 |  15.4 |
|         00:00:09.296 | src: [fdde:ad00:beef:0:0:ff:fe00:800]:0                                     |
|                      | dst: [ff03:0:0:0:0:0:0:2]:0                                                 |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |    50 | 0xc1d8 |  no |  net |  -20 | RX | 0x0800 |  15.4 |
|         00:00:09.569 | src: [fe80:0:0:0:54d9:5153:ffc6:df26]:19788                                 |
|                      | dst: [ff02:0:0:0:0:0:0:1]:19788                                             |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |    50 | 0x3cb1 |  no |  net |  -20 | RX | 0x0800 |  15.4 |
|         00:00:16.519 | src: [fe80:0:0:0:54d9:5153:ffc6:df26]:19788                                 |
|                      | dst: [ff02:0:0:0:0:0:0:1]:19788                                             |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |    50 | 0xeda0 |  no |  net |  -20 | RX | 0x0800 |  15.4 |
|         00:00:20.599 | src: [fe80:0:0:0:54d9:5153:ffc6:df26]:19788                                 |
|                      | dst: [ff02:0:0:0:0:0:0:1]:19788                                             |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |   165 | 0xbdfa | yes |  net |  -20 | RX | 0x0800 |  15.4 |
|         00:00:21.059 | src: [fe80:0:0:0:54d9:5153:ffc6:df26]:19788                                 |
|                      | dst: [fe80:0:0:0:8893:c2cc:d983:1e1c]:19788                                 |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |    64 | 0x1c11 |  no |  net |  NA  | TX | 0x0800 |  15.4 |
|         00:00:21.062 | src: [fe80:0:0:0:8893:c2cc:d983:1e1c]:19788                                 |
|                      | dst: [fe80:0:0:0:54d9:5153:ffc6:df26]:19788                                 |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |    93 | 0xedff |  no |  net |  -20 | RX | unknwn |  15.4 |
|         00:00:21.474 | src: [fe80:0:0:0:54d9:5153:ffc6:df26]:19788                                 |
|                      | dst: [fe80:0:0:0:8893:c2cc:d983:1e1c]:19788                                 |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |    44 | 0xd383 |  no |  net |  NA  | TX | bcast  |  15.4 |
|         00:00:21.811 | src: [fe80:0:0:0:8893:c2cc:d983:1e1c]:19788                                 |
|                      | dst: [ff02:0:0:0:0:0:0:2]:19788                                             |
Done
```

Print the latest 5 entries of the IPv6 message RX history as a list:

```bash
> history rxtx list 5

00:00:02.100
    type:UDP len:50 checksum:0xd843 sec:no prio:net rss:-20 from:0x0800 radio:15.4
    src:[fe80:0:0:0:54d9:5153:ffc6:df26]:19788
    dst:[ff02:0:0:0:0:0:0:1]:19788
00:00:15.331
    type:HopOpts len:44 checksum:0x0000 sec:yes prio:norm rss:-20 from:0x0800 radio:15.4
    src:[fdde:ad00:beef:0:0:ff:fe00:800]:0
    dst:[ff03:0:0:0:0:0:0:2]:0
00:00:15.354
    type:UDP len:12 checksum:0x6c6b sec:yes prio:net rss:-20 from:0x0800 radio:15.4
    src:[fdde:ad00:beef:0:0:ff:fe00:800]:61631
    dst:[fdde:ad00:beef:0:0:ff:fe00:801]:61631
00:00:15.356
    type:ICMP6(EchoReqst) len:16 checksum:0xc6a2 sec:yes prio:norm rss:-20 from:0x0800 radio:15.4
    src:[fdde:ad00:beef:0:efe8:4910:cf95:dee9]:0
    dst:[fdde:ad00:beef:0:af4c:3644:882a:3698]:0
00:00:15.356
    type:ICMP6(EchoReply) len:16 checksum:0xc5a2 sec:yes prio:norm tx-success:yes to:0x0800 radio:15.4
    src:[fdde:ad00:beef:0:af4c:3644:882a:3698]:0
    dst:[fdde:ad00:beef:0:efe8:4910:cf95:dee9]:0
```

### tx

Usage `history tx [list] [<num-entries>]`

Print the IPv6 message TX history in either table or list format. Entries provide same information and follow same format as in `history rxtx` command.

Print the IPv6 message TX history as a table (10 latest entries):

```bash
> history tx
| Age                  | Type             | Len   | Chksum | Sec | Prio | RSS  |Dir | Neighb | Radio |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | ICMP6(EchoReply) |    16 | 0x932c | yes | norm |  NA  | TX | 0x4800 |  15.4 |
|         00:00:18.798 | src: [fdde:ad00:beef:0:dc0e:d6b3:f180:b75b]:0                               |
|                      | dst: [fdde:ad00:beef:0:ac09:a16b:3204:dc09]:0                               |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |    50 | 0xce87 | yes |  net |  NA  | TX | 0x4800 |  15.4 |
|         00:00:18.800 | src: [fdde:ad00:beef:0:0:ff:fe00:4801]:61631                                |
|                      | dst: [fdde:ad00:beef:0:0:ff:fe00:4800]:61631                                |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |    64 | 0xf7ba |  no |  net |  NA  | TX | 0x4800 |  15.4 |
|         00:00:39.499 | src: [fe80:0:0:0:a4a5:bbac:a8e:bd07]:19788                                  |
|                      | dst: [fe80:0:0:0:d03d:d3e7:cc5e:7cd7]:19788                                 |
+----------------------+------------------+-------+--------+-----+------+------+----+--------+-------+
|                      | UDP              |    44 | 0x26d4 |  no |  net |  NA  | TX | bcast  |  15.4 |
|         00:00:40.256 | src: [fe80:0:0:0:a4a5:bbac:a8e:bd07]:19788                                  |
|                      | dst: [ff02:0:0:0:0:0:0:2]:19788                                             |
Done
```

Print the IPv6 message TX history as a list:

```bash
history tx list
00:00:23.957
    type:ICMP6(EchoReply) len:16 checksum:0x932c sec:yes prio:norm tx-success:yes to:0x4800 radio:15.4
    src:[fdde:ad00:beef:0:dc0e:d6b3:f180:b75b]:0
    dst:[fdde:ad00:beef:0:ac09:a16b:3204:dc09]:0
00:00:23.959
    type:UDP len:50 checksum:0xce87 sec:yes prio:net tx-success:yes to:0x4800 radio:15.4
    src:[fdde:ad00:beef:0:0:ff:fe00:4801]:61631
    dst:[fdde:ad00:beef:0:0:ff:fe00:4800]:61631
00:00:44.658
    type:UDP len:64 checksum:0xf7ba sec:no prio:net tx-success:yes to:0x4800 radio:15.4
    src:[fe80:0:0:0:a4a5:bbac:a8e:bd07]:19788
    dst:[fe80:0:0:0:d03d:d3e7:cc5e:7cd7]:19788
00:00:45.415
    type:UDP len:44 checksum:0x26d4 sec:no prio:net tx-success:yes to:0xffff radio:15.4
    src:[fe80:0:0:0:a4a5:bbac:a8e:bd07]:19788
    dst:[ff02:0:0:0:0:0:0:2]:19788
Done
```
