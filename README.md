# RACSHELL

Open source RACF shell interface intended for z/OS Unix SSH sessions

## Commands

The following commands are currently implemented in `racshell/`.

### Command Overview

| Command | Category | Purpose | Key Options |
| --- | --- | --- | --- |
| `adduser` | User | Create RACF user | `--traits`, `--debug`, `--all-json`, `--no-color` |
| `altuser` | User | Alter RACF user | `--traits`, `--debug`, `--all-json`, `--no-color` |
| `deleteuser` | User | Delete RACF user | `--debug`, `--all-json`, `--no-color` |
| `listuser` | User | Display RACF user details | `--groups`, `--tso`, `--kerberos`, `--cics`, `--omvs`, `--security`, `--json`, `--all-json`, `--debug`, `--no-color` |
| `addgroup` | Group | Create RACF group | `--traits`, `--debug`, `--all-json`, `--no-color` |
| `deletegroup` | Group | Delete RACF group | `--debug`, `--all-json`, `--no-color` |
| `listgrp` | Group | Display RACF group details | `--users`, `--omvs`, `--json`, `--all-json`, `--debug`, `--no-color` |

## User Commands

User administration and lookup commands.

### adduser

Create a RACF user.

Usage:

`adduser <userid> [options]`

Options:

- `-t, --traits` traits to set (repeatable), e.g. `base:name='John Doe' omvs:uid=24`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-a, --all-json` output full raw SEAR JSON response

### altuser

Alter an existing RACF user.

Usage:

`altuser <userid> [options]`

Options:

- `-t, --traits` traits to alter (repeatable), e.g. `base:name='LEONARD CARCARAMO' base:special=true`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-a, --all-json` output full raw SEAR JSON response

### deleteuser

Delete a RACF user.

Usage:

`deleteuser <userid> [options]`

Options:

- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-a, --all-json` output full raw SEAR JSON response

### listuser

Extract and display RACF user details.

Usage:

`listuser <user> [options]`

Options:

- `-g, --groups` list connected RACF groups
- `-t, --tso` list TSO segment
- `-k, --kerberos` list Kerberos segment
- `-c, --cics` list CICS segment
- `-o, --omvs` list OMVS segment
- `-s, --security` list security-related fields
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### Group Commands

Group administration and lookup commands.

### addgroup

Create a RACF group.

Usage:

`addgroup <group> [options]`

Options:

- `-t, --traits` traits to set (repeatable), e.g. `base:owner=SYS1 base:superior_group=SYS1`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-a, --all-json` output full raw SEAR JSON response

### deletegroup

Delete a RACF group.

Usage:

`deletegroup <group> [options]`

Options:

- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-a, --all-json` output full raw SEAR JSON response

### listgrp

Extract and display RACF group details.

Usage:

`listgrp <group> [options]`

Options:

- `-u, --users` list connected users
- `-o, --omvs` list OMVS segment
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response
