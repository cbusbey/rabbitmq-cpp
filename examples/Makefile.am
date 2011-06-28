include ../Common.mk

noinst_PROGRAMS=connect producer consumer exchange_declare

AM_CXXFLAGS+=-I$(top_builddir)/src
AM_LDFLAGS+=-L$(top_builddir)/src -lrabbitmq_cpp -lboost_thread

connect_SOURCES=connect.cc
producer_SOURCES=producer.cc
consumer_SOURCES=consumer.cc
exchange_declare_SOURCES=exchange_declare.cc
