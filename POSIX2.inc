LOGGING_H = Zaimoni.STL/Logging.h Zaimoni.STL/Compiler.h
DICOUNTER_HPP = dicounter.hpp $(LOGGING_H)
ANGLE_HPP = angle.hpp $(LOGGING_H) 
AUGMENT_CMATH = Zaimoni.STL/augment.STL/cmath Zaimoni.STL/augment.STL/type_traits $(LOGGING_H)
INTERVAL_HPP = interval.hpp $(AUGMENT_CMATH)
LOSSY_HPP = lossy.hpp $(AUGMENT_CMATH)
CONSTANTS_HPP = constants.hpp $(INTERVAL_HPP)
MASS_HPP = mass.hpp $(CONSTANTS_HPP)
Z_N_HPP = z_n.hpp Zaimoni.STL/augment.STL/type_traits $(LOGGING_H)
INT_RANGE_HPP = int_range.hpp Zaimoni.STL/augment.STL/type_traits $(LOGGING_H)
CYCLIC_FN_HPP = cyclic_fn.hpp $(Z_N_HPP)
OVERPRECISE_HPP = overprecise.hpp $(DICOUNTER_HPP) $(INT_RANGE_HPP) $(LOSSY_HPP) Zaimoni.STL/Pure.C/auto_int.h
SERIES_SUM_HPP = series_sum.hpp $(OVERPRECISE_HPP)
TAYLOR_HPP = taylor.hpp Zaimoni.STL/augment.STL/functional $(SERIES_SUM_HPP)
EUCLIDEAN_HPP = Euclidean.hpp $(SERIES_SUM_HPP)
SLICE_HPP = slice.hpp Zaimoni.STL/iterator_array_size.hpp
MATRIX_HPP = matrix.hpp $(LOGGING_HPP) Zaimoni.STL/rw.hpp $(SLICE_HPP) $(EUCLIDEAN_HPP)

TESTDRIVER_LIBS = -lz_log_adapter -lz_stdio_log -lz_format_util
TESTDRIVER_FLAGS =  -Os -D__STDC_LIMIT_MACROS

angle.test.exe: angle.cpp $(ANGLE_HPP) $(TAYLOR_HPP)
	$(CXX) -oangle.test.exe $(TESTDRIVER_FLAGS) -DTEST_APP2 angle.cpp taylor.cpp $(LINK_FLAGS) $(TESTDRIVER_LIBS)
	strip --strip-unneeded @$
	@$

constants.exe: constants.cpp $(CONSTANTS_HPP)
	$(CXX) -oconstants.exe $(TESTDRIVER_FLAGS) -DTEST_APP constants.cpp $(LINK_FLAGS) -lz_stdio_c $(TESTDRIVER_LIBS)
	strip --strip-unneeded $@
	$@

cyclic_fn.test.exe: cyclic_fn.cpp $(CYCLIC_FN_HPP)
	$(CXX) -ocyclic_fn.test.exe $(TESTDRIVER_FLAGS) cyclic_fn.cpp $(LINK_FLAGS) $(TESTDRIVER_LIBS)
	strip --strip-unneeded $@
	$@

dicounter.test.exe: dicounter.cpp dicounter.hpp $(LOGGING_H)
	$(CXX) -odicounter.test.exe $(TESTDRIVER_FLAGS) dicounter.cpp $(LINK_FLAGS) $(TESTDRIVER_LIBS)
	strip --strip-unneeded $@
	$@

int_range.test.exe: int_range.cpp $(INT_RANGE_HPP)
	$(CXX) -oint_range.test.exe $(TESTDRIVER_FLAGS) int_range.cpp $(LINK_FLAGS) $(TESTDRIVER_LIBS)
	strip --strip-unneeded $@
	$@

interval.test.exe: interval.cpp $(INTERVAL_HPP)
	$(CXX) -ointerval.test.exe $(TESTDRIVER_FLAGS) interval.cpp $(LINK_FLAGS) $(TESTDRIVER_LIBS)
	strip --strip-unneeded $@
	$@

lossy.test.exe: lossy.cpp $(LOSSY_HPPH)
	$(CXX) -olossy.test.exe $(TESTDRIVER_FLAGS) lossy.cpp $(LINK_FLAGS) $(TESTDRIVER_LIBS)
	strip --strip-unneeded $@
	$@

mass.test.exe: mass.cpp $(MASS_HPP)
	$(CXX) -omass.test.exe $(TESTDRIVER_FLAGS) -DTEST_APP2 mass.cpp constants.cpp $(LINK_FLAGS) -lz_stdio_c $(TESTDRIVER_LIBS)
	strip --strip-unneeded $@
	$@

matrix.test.exe: matrix.cpp $(MATRIX_HPP)
	$(CXX) -omatrix.test.exe $(TESTDRIVER_FLAGS) matrix.cpp $(LINK_FLAGS) $(TESTDRIVER_LIBS)
	strip --strip-unneeded $@
	$@

taylor.test.exe: taylor.cpp $(TAYLOR_HPP) $(CYCLIC_FN_HPP)
	$(CXX) -otaylor.test.exe $(TESTDRIVER_FLAGS) -DTEST_APP taylor.cpp $(LINK_FLAGS) $(TESTDRIVER_LIBS)
	strip --strip-unneeded $@
	$@

check: angle.test.exe constants.exe cyclic_fn.test.exe dicounter.test.exe int_range.test.exe interval.test.exe lossy.test.exe mass.test.exe matrix.test.exe taylor.test.exe