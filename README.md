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
| `altgroup` | Group | Alter RACF group | `--traits`, `--debug`, `--all-json`, `--no-color` |
| `deletegroup` | Group | Delete RACF group | `--debug`, `--all-json`, `--no-color` |
| `connect` | Group | Connect user to RACF group | `--traits`, `--debug`, `--all-json`, `--no-color` |
| `remove` | Group | Remove user from RACF group | `--debug`, `--all-json`, `--no-color` |
| `listgroup` | Group | Display RACF group details | `--users`, `--omvs`, `--json`, `--all-json`, `--debug`, `--no-color` |
| `adddataset` | Dataset | Create RACF dataset profile | `--generic`, `--traits`, `--debug`, `--all-json`, `--no-color` |
| `deletedataset` | Dataset | Delete RACF dataset profile | `--generic`, `--debug`, `--all-json`, `--no-color` |
| `listdataset` | Dataset | Display RACF dataset profile details | `--generic`, `--debug`, `--all-json`, `--no-color` |

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

### altgroup

Alter an existing RACF group.

Usage:

`altgroup <group> [options]`

Options:

- `-t, --traits` traits to alter (repeatable), e.g. `base:owner=SYS1 omvs:gid=1234567`
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

### listgroup

Extract and display RACF group details.

Usage:

`listgroup <group> [options]`

Options:

- `-u, --users` list connected users
- `-o, --omvs` list OMVS segment
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-j, --json` output as JSON
- `-a, --all-json` output full raw SEAR JSON response

### connect

Connect a RACF user to a RACF group.

Usage:

`connect <userid> <group> [options]`

Options:

- `-t, --traits` connection traits to set (repeatable), e.g. `base:special=true`
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-a, --all-json` output full raw SEAR JSON response

### remove

Remove a RACF user from a RACF group.

Usage:

`remove <userid> <group> [options]`

Options:

- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-a, --all-json` output full raw SEAR JSON response

## Dataset Commands

Dataset profile administration commands.

### adddataset

Create a RACF dataset profile.

Usage:

`adddataset <dataset> [options]`

Options:

- `-t, --traits` traits to set (repeatable), e.g. `base:universal_access=None base:owner=eswift`
- `-g, --generic` treat the dataset name as a generic profile (e.g. `ESWIFT.TEST.**`)
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-a, --all-json` output full raw SEAR JSON response

### deletedataset

Delete a RACF dataset profile.

Usage:

`deletedataset <dataset> [options]`

Options:

- `-g, --generic` treat the dataset name as a generic profile
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-a, --all-json` output full raw SEAR JSON response

### listdataset

Extract and display RACF dataset profile details.

Usage:

`listdataset <dataset> [options]`

Options:

- `-g, --generic` treat the dataset name as a generic profile
- `-n, --no-color` disable colored output
- `-d, --debug` debug SEAR request and response
- `-a, --all-json` output full raw SEAR JSON response


