//    Copyright 2019 Andreu Carminati
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.

#include <cstdio>
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/// putchard - putchar that takes a double and returns 0.
extern "C" DLLEXPORT void putchard(long long X) {
  fputc((char)X, stderr);
}

/// printreal - printf that takes a double prints it as "%f\n".
extern "C" DLLEXPORT void printreal(double X) {
  fprintf(stderr, "%f\n", X);
}

/// printinteger - printf that takes an integer prints it as "%lld\n".
extern "C" DLLEXPORT void printinteger(long long X) {
  fprintf(stderr, "%lld\n", X);
}
