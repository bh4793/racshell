# RACSHELL

Open source RACF shell interface intended for z/OS Unix SSH sessions

## Commands

The following commands are currently implemented in `racshell/`.

### Command Overview

| Command | Category | Purpose | Key Options |
| --- | --- | --- | --- |
| `adduser` | User | Create RACF user | `--trait`, `--debug`, `--all-json`, `--no-color` |
| `altuser` | User | Alter RACF user | `--trait`, `--debug`, `--all-json`, `--no-color` |
| `deleteuser` | User | Delete RACF user | `--debug`, `--all-json`, `--no-color` |
| `listuser` | User | Display RACF user details | `--groups`, `--tso`, `--kerberos`, `--cics`, `--omvs`, `--security`, `--json`, `--all-json`, `--debug`, `--no-color` |
| `listgrp` | Group | Display RACF group details | `--users`, `--omvs`, `--json`, `--all-json`, `--debug`, `--no-color` |

## User Commands

User administration and lookup commands.

### adduser

Create a RACF user.

Usage:

`adduser <userid> [options]`

Options:

- `-t, --trait` trait to set (repeatable), e.g. `base:name='John Doe' omvs:uid=24`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-a, --all-json` output full raw SEAR JSON response

### altuser

Alter an existing RACF user.

Usage:

`altuser <userid> [options]`

Options:

- `-t, --trait` trait to alter (repeatable), e.g. `base:name='LEONARD CARCARAMO' base:special=true`
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
