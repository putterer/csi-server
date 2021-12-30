# CSI-Server
A server for shipping channel state information obtained using the Atheros CSI Tool via UDP in real time.

The CSI Server was written for usage in combination with an OpenWRT based embedded
device but should be capable of being executed on a x86 based system as well. It is based
upon the code of the [Atheros CSI tool’s](https://wands.sg/research/wifi/AtherosCSI/) recvCSI user space app.

It also supports obtaining CSI using the Intel IWL5300 using the [Linux CSI tool](https://dhalperi.github.io/linux-80211n-csitool/). The tool is not integrated with this application, you can just pipe the output of the Linux CSI tool's `log_to_file` executable into the `stdin` of this application which will cause it to treat the received CSI data the same way it would using the Atheros CSI tool.

## Prebuilt binary
A prebuilt binary for the MIPS architecture can be found at [https://github.com/putterer/csi-auxiliary/tree/main/router](https://github.com/putterer/csi-auxiliary/tree/main/router).

## Building
The project provides a Makefile that by default cross compiles the program for run-
ning one OpenWRT, which can be used after the folder containing the `mips-openwrt--
linux-gcc` executable file, which can be found in the `staging_dir` after the OpenWRT
firmware has been built, was added to the `PATH`.

```make```

To build the software for x86, simply run:

```make x86```

## Operation
The entire communication uses UDP. Clients are able to (un)subscribe to the server by
sending it a subscription message containing filter options. At the moment, the only
filter that is supported is the payload length of the received packet for which CSI is to be
calculated. Set to 0 to ignore.

The CSI Server reads information about received CSI values from the `/dev/CSI_dev`
kernel device of the Atheros CSI tool or from `stdin` for the Intel CSI tool. It then packages this information and forwards it to all
subscribed devices with applicable filter options using UDP.
