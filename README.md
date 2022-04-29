
*OVerview*

Write an XDR stream to a Ceph rados object.

*Configuring*

You will need a rados pool named work and also a ceph configuration file, defaulting to /etc/ceph/ceph_ipv4.conf

*Compiling and running*

```console
make rados_xdr
./rados_xdr
```

*Decoding stream*

```console
rados --namespace chessdb -p work get encoded_stream.xdr encoded_stream.xdr
make show-stream
./show-stream ./encoded_stream.xdr
```

