# switch configuration

This doc describes how a postmerkos switch is configured.

> **NOTE**: If you are an end-user looking to manage your switch, we recommend the [UI](#UI).
> The rest of this doc largely goes into detail about how all of the parts are stitched together under the hood.

## click

The underlying interface is a filesystem mounted at `/click`.

> **TODO**: are there any docs we can reference?

If you're developing code to run directly on the switch, you may need to go through `click`.
All I can say is: search the repo for some examples.

Most common settings can be modified through one of the more user-friendly options described below.

## json

The JSON file at `/etc/switch.json` is intended to be the source of truth for user-level configuration.

It should define everything most users will need (if not, please open an issue).
You _can_ edit this file directly but there are other options, described in the sections below, which provide features such as type-checking and validation.

### schema

The [JSON Schema](https://json-schema.org/) for this file is defined in [`config.schema.json`](./schema/config.schema.json).

An example, truncated to include only 2 ports:

```json
{
  "ports": {
    "1": {
      "enabled": true,
      "poe": {
        "enabled": true,
        "mode": "802.3at"
      }
    },
    "2": {
      "enabled": true,
      "poe": {
        "enabled": false,
        "mode": "802.3at"
      }
    }
  }
}
```

## bin

There are a few utilities available on the switch that prove useful when interacting with a `switch.json` file.

- `switch_status` can be used to generate a `switch.json` file
- [`jshn`](https://openwrt.org/docs/guide-developer/jshn) is a JSON library for bash scripting

## daemon

The `configd` daemon will poll (with a 10 second wait) the `/etc/switch.json` file for changes and apply them to the switch using the `/click` interface.

## API

CGI scripts served by uhttpd on port 80 will respond to the following requests.

The base URL is

    http://{switch}/cgi-bin

Authentication is provided by PAM (i.e., the `root` user's login credentials).

### config

- Get the current configuration

      GET /config

- Upload a new configuration

      POST /config
      content-type: application/json

      {
          "ports": {
              "1": {. . .},
              "2": {. . .},
              . . .
          }
      }

### status

- Get the current status

      GET /status

  An example response:

  ```json
  {
    "datetime": "2022-01-21T18:58:15.837Z",
    "temperature": {
      "system": [
        {
          "zone": 0,
          "temp": 32350
        }
      ],
      "poe": [
        {
          "temp": 25400
        }
      ]
    },
    "ports": {
      "1": {
        "link": {
          "established": true,
          "speed": 1000
        },
        "poe": {
          "power": 6
        }
      }
    }
  }
  ```

## UI

A graphical interface is available at http://{switch}.
Here, you will find the status and configuration options exposed in an easy-to-use management dashboard.

All of the configuration found in the sections above (except for the `click` filesystem) can be managed through the UI.
