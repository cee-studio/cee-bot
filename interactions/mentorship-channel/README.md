# Interaction - `Create Mentorship Channel`

## Channel creation menu

Before you get started, make sure to locate the `cee_bot` field of your bot's `config.json`, and
update it with your guild primitives.

Once you are ready, you can come back to this folder and build its source files:
```bash
$ make
```

### `menu-set.c` - Initialize the select menu message to guild's `#rules` channel

* Modify `menu.json` to change how the select menu interaction is structured
* Run `./menu-set`

### `menu-react.c` - React to select menu user interaction

This file consists of the react logic for this interaction, it is included at `main.c`.

## Commands for mentorship channel editing

### `cmd-set.c` - Set the application commands the users may use for editing their channel

* Modify `cmds/` JSON files to change how the application commands are structured
* Run `./cmd-set`
