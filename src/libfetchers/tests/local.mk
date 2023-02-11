check: libfetchers-tests_RUN

programs += libfetchers-tests

libfetchers-tests_NAME := libnixfetchers-tests

libfetchers-tests_DIR := $(d)

libfetchers-tests_INSTALL_DIR :=

libfetchers-tests_SOURCES := \
    $(wildcard $(d)/*.cc)

libfetchers-tests_CXXFLAGS += -I src/libfetchers -I src/libutil -I src/libstore -I src/libfetchers/tests

libfetchers-tests_LIBS = libstore-tests libutils-tests libfetchers libutil libstore

libfetchers-tests_LDFLAGS := $(GTEST_LIBS) -lgmock -lgit2
