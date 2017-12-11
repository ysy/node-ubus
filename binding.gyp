{
  "targets": [
    {
      "target_name": "ubus_bindings",
      "sources": [ "ubus.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ],
      'libraries': [ '-lubus', '-ljson-c', '-lblobmsg_json', '-lubox' ]
    }
  ]
}
