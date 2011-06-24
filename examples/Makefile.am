include ../Common.mk

noinst_PROGRAMS=connect producer consumer

AM_CXXFLAGS+=-I$(top_builddir)/src
AM_LDFLAGS+=-L$(top_builddir)/src -lrabbitmq_cpp

connect_SOURCES=connect.cc
producer_SOURCES=producer.cc
consumer_SOURCES=consumer.cc
