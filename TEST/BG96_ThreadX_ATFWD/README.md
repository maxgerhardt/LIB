
Firmware: **BG96MAR03A03M1G_01.002.01.002**

SDK: version **3.2.5**

Log

```
RDY
ATIQuectel
BG96
Revision: BG96MAR03A03M1G

OK
AT+QEXAMPLEOK
AT+QEXAMPLE?
+QEXAMPLE: 0
OK

```

```
ThreadX BG96 Hello World
AT tx_byte_pool_create DONE
qapi_atfwd_Pass_Pool_Ptr DONE
qapi_atfwd_reg  DONE
atfwd_cmd_handler_cb is called, atcmd_name:[+QEXAMPLE] mask:[0]
atfwd_cmd_handler_cb is called, is_reg:[1]
Atcmd +QEXAMPLE is registered
atfwd_cmd_handler_cb is called, atcmd_name:[+QEXAMPLE] mask:[1]
atfwd_cmd_handler_cb is called, is_reg:[1]
[+QEXAMPLE] send resp, ret = 0
atfwd_cmd_handler_cb is called, atcmd_name:[+QEXAMPLE] mask:[5]
atfwd_cmd_handler_cb is called, is_reg:[1]
[+QEXAMPLE] send resp, ret = 0

```
