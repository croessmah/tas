TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = \
          tas \
          tas_ipc \
          eye

eye.depends = tas_ipc

