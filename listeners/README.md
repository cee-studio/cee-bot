# Interaction Listeners

## Activate interaction listeners in your guild

Before you get started, make sure to locate the `cee_bot` field of your bot's `config.json`, and
update it with your guild primitives.

Once you are ready, you can come back to this folder and continue:

1. Build executables:
```bash
$ make
```
2. (optional) Modify the default listeners by editing the sub-folder's JSON files.
3. Run executables to activate their respective listeners

# Listeners Listing

## [mentorship-channel/](mentorship-channel/)

### Channel creation menu

#### `menu.c` - Initialize the select menu message to guild's `#rules` channel

* Modify [`menu.json`](mentorship-channel/menu.json) to change how the select menu interaction is structured
* Run `./menu`

### Commands for mentorship channel editing

#### `cmd.c` - Set the application commands the users may use for editing their channel

* Modify [`cmd.json`](mentorship-channel/cmd.json) to change how the mentorship channel application commands are structured
* Run `./cmd`
