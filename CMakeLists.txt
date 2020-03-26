cmake_minimum_required(VERSION 3.4.3)
project(JIT-Interpreter)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.
set ( CMAKE_CXX_FLAGS "-fno-rtti -g")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Now build our tools
add_executable(interpreter src/Executor.cpp src/LLVMIRGen.cpp src/Lexer.cpp src/Optimizer.cpp src/Runtime.cpp src/SymbolTable.cpp src/Main.cpp src/MLIRGen.cpp)

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader instcombine orcjit X86 x86codegen x86info)

set(LIBS
${dialect_libs}
${conversion_libs}
MLIRLoopOpsTransforms
MLIRLoopAnalysis
MLIRAnalysis
MLIRDialect
MLIREDSC
MLIROptLib
MLIRParser
MLIRPass
MLIRQuantizerFxpMathConfig
MLIRQuantizerSupport
MLIRQuantizerTransforms
MLIRSPIRV
MLIRSPIRVTestPasses
MLIRSPIRVTransforms
MLIRTransforms
MLIRTransformUtils
MLIRTestDialect
MLIRTestIR
MLIRTestPass
MLIRTestTransforms
MLIRSupport
MLIRIR
MLIROptLib
)

# Link against LLVM libraries
target_link_libraries(interpreter ${LIBS} ${llvm_libs} -lstdc++ -lpthread -ltinfo -rdynamic -ldl -lz)
