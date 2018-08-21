TEMPLATE = subdirs

3rdparty-qamqp.file = 3rdparty/qamqp/qamqp-proofed.pro
core.file = core.pro
network.file = network.pro
network.depends = 3rdparty-qamqp core

SUBDIRS = 3rdparty-qamqp core network
