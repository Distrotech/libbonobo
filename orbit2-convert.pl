#!/usr/bin/perl -pi.bak

# This script is designed to convert your ORBit1 source to
# ORBit2 compatible source, use:
#
# find -name '*.c' -exec /path/to/orbit2-convert.pl {} \;
# find -name '*.h' -exec /path/to/orbit2-convert.pl {} \;
#

# This section is forward and backwards compatible:

	s/TC_short/TC_CORBA_short/g;
	s/TC_long/TC_CORBA_long/g;
	s/TC_longlong/TC_CORBA_long_long/g;
	s/TC_ushort/TC_CORBA_unsigned_short/g;
	s/TC_ulong/TC_CORBA_unsigned_long/g;
	s/TC_ulonglong/TC_CORBA_unsigned_long_long/g;
	s/TC_float/TC_CORBA_float/g;
	s/TC_double/TC_CORBA_double/g;
	s/TC_longdouble/TC_CORBA_long_double/g;
	s/TC_boolean/TC_CORBA_boolean/g;
	s/TC_char/TC_CORBA_char/g;
	s/TC_wchar/TC_CORBA_wchar/g;
	s/TC_octet/TC_CORBA_octet/g;
	s/TC_any/TC_CORBA_any/g;
	s/TC_TypeCode/TC_CORBA_TypeCode/g;
	s/TC_Principal/TC_CORBA_Principal/g;
	s/TC_Object/TC_CORBA_Object/g;
	s/TC_string/TC_CORBA_string/g;
	s/TC_wstring/TC_CORBA_wstring/g;
	s/TC_CORBA_NamedValue/TC_CORBA_CORBA_NamedValue/g;

# some incompatible changes

	s/CORBA_sequence_octet/CORBA_sequence_CORBA_octet/g;
