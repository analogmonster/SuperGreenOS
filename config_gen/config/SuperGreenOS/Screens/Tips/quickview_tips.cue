package config

modules quickview_tips: _MODULE & {
  init_priority: 30
}

modules quickview_tips fields "order": _UINT8 & _NVS & _HTTP_RW & {
  default: 0
  nvs key: "QVSCR_ORDR"
  write_cb: true
}

