TEMPLATE = subdirs

SUBDIRS = common \
          browser \
          service

browser.depends = common
service.depends = common
