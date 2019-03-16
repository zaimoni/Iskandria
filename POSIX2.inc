LOGGING_H = Zaimoni.STL/Logging.h Zaimoni.STL/Compiler.h
DICOUNTER_HPP = dicounter.hpp $(LOGGING_H)
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

constants.exe: constants.cpp $(CONSTANTS_HPP)
	g++ -std=c++14 -oconstants.exe -D__STDC_LIMIT_MACROS -DTEST_APP constants.cpp -Llib\host.isk -lz_stdio_c -lz_log_adapter -lz_stdio_log -lz_format_util
	constants.exe

cyclic_fn.test.exe: cyclic_fn.cpp $(CYCLIC_FN_HPP)
	g++ -std=c++14 -ocyclic_fn.test.exe cyclic_fn.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util
	cyclic_fn.test.exe

dicounter.test.exe: dicounter.cpp dicounter.hpp $(LOGGING_H)
	g++ -std=c++14 -odicounter.test.exe -D__STDC_LIMIT_MACROS dicounter.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util
	dicounter.test.exe

int_range.test.exe: int_range.cpp $(INT_RANGE_HPP)
	g++ -std=c++14 -oint_range.test.exe int_range.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util
	int_range.test.exe

lossy.test.exe: lossy.cpp lossy.hpp
	g++ -std=c++14 -olossy.test.exe lossy.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util
	lossy.test.exe

mass.test.exe: mass.cpp $(MASS_HPP)
	g++ -std=c++14 -omass.test.exe -D__STDC_LIMIT_MACROS -DTEST_APP2 mass.cpp constants.cpp -Llib\host.isk -lz_stdio_c -lz_log_adapter -lz_stdio_log -lz_format_util
	mass.test.exe

taylor.test.exe: taylor.cpp $(TAYLOR_HPP) $(CYCLIC_FN_HPP)
	g++ -std=c++14 -otaylor.test.exe -DTEST_APP -D__STDC_LIMIT_MACROS taylor.cpp -Llib/host.isk -lz_log_adapter -lz_stdio_log -lz_format_util
	taylor.test.exe

check: constants.exe cyclic_fn.test.exe dicounter.test.exe int_range.test.exe lossy.test.exe mass.test.exe taylor.test.exe