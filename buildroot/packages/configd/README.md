# configd

A status and configuration daemon to bridge interfaces.

In particular, when run as a daemon (`config -d`), it will read the
[click](../click/) interface and write user-configurable parameters to
`/etc/switch.json`. It will then watch for changes to that file and apply them
back through click.

It can also be used to fetch status information (`config -g`) to populate other
interfaces (e.g., the web UI or a metrics dashboard).
