/*
 * Copyright (c) 2015, 2018, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#include "/workspace/spring-petclinic/openjdk/test/hotspot/gtest/jfr/precompiled.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/code/relocInfo.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/compiler/compilerDefinitions.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/oops/metadata.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/os.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/interpreter/invocationCounter.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/arguments.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/flags/jvmFlag.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/flags/jvmFlagConstraintsCompiler.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/globals.hpp"
#include "/workspace/spring-petclinic/openjdk/src/hotspot/share/runtime/globals_extension.hpp"

JVMFlag::Error AliasLevelConstraintFunc(intx value, bool verbose) {
  if ((value <= 1) && (Arguments::mode() == Arguments::_comp || Arguments::mode() == Arguments::_mixed)) {
    JVMFlag::printError(verbose,
                        "AliasLevel (" INTX_FORMAT ") is not "
                        "compatible with -Xcomp or -Xmixed\n",
                        value);
    return JVMFlag::VIOLATES_CONSTRAINT;
  } else {
    return JVMFlag::SUCCESS;
  }
}

/**
 * Validate the minimum number of compiler threads needed to run the
 * JVM. The following configurations are possible.
 *
 * 1) The JVM is build using an interpreter only. As a result, the minimum number of
 *    compiler threads is 0.
 * 2) The JVM is build using the compiler(s) and tiered compilation is disabled. As
 *    a result, either C1 or C2 is used, so the minimum number of compiler threads is 1.
 * 3) The JVM is build using the compiler(s) and tiered compilation is enabled. However,
 *    the option "TieredStopAtLevel < CompLevel_full_optimization". As a result, only
 *    C1 can be used, so the minimum number of compiler threads is 1.
 * 4) The JVM is build using the compilers and tiered compilation is enabled. The option
 *    'TieredStopAtLevel = CompLevel_full_optimization' (the default value). As a result,
 *    the minimum number of compiler threads is 2.
 */
JVMFlag::Error CICompilerCountConstraintFunc(intx value, bool verbose) {
  int min_number_of_compiler_threads = 0;
#if !defined(COMPILER1) && !defined(COMPILER2) && !INCLUDE_JVMCI
  // case 1
#else
  if (!TieredCompilation || (TieredStopAtLevel < CompLevel_full_optimization)) {
    min_number_of_compiler_threads = 1; // case 2 or case 3
  } else {
    min_number_of_compiler_threads = 2;   // case 4 (tiered)
  }
#endif

  // The default CICompilerCount's value is CI_COMPILER_COUNT.
  // With a client VM, -XX:+TieredCompilation causes TieredCompilation
  // to be true here (the option is validated later) and
  // min_number_of_compiler_threads to exceed CI_COMPILER_COUNT.
  min_number_of_compiler_threads = MIN2(min_number_of_compiler_threads, CI_COMPILER_COUNT);

  if (value < (intx)min_number_of_compiler_threads) {
    JVMFlag::printError(verbose,
                        "CICompilerCount (" INTX_FORMAT ") must be "
                        "at least %d \n",
                        value, min_number_of_compiler_threads);
    return JVMFlag::VIOLATES_CONSTRAINT;
  } else {
    return JVMFlag::SUCCESS;
  }
}

JVMFlag::Error AllocatePrefetchDistanceConstraintFunc(intx value, bool verbose) {
  if (value < 0 || value > 512) {
    JVMFlag::printError(verbose,
                        "AllocatePrefetchDistance (" INTX_FORMAT ") must be "
                        "between 0 and " INTX_FORMAT "\n",
                        AllocatePrefetchDistance, 512);
    return JVMFlag::VIOLATES_CONSTRAINT;
  }

  return JVMFlag::SUCCESS;
}

JVMFlag::Error AllocatePrefetchStepSizeConstraintFunc(intx value, bool verbose) {
  if (AllocatePrefetchStyle == 3) {
    if (value % wordSize != 0) {
      JVMFlag::printError(verbose,
                          "AllocatePrefetchStepSize (" INTX_FORMAT ") must be multiple of %d\n",
                          value, wordSize);
      return JVMFlag::VIOLATES_CONSTRAINT;
    }
  }
  return JVMFlag::SUCCESS;
}

JVMFlag::Error AllocatePrefetchInstrConstraintFunc(intx value, bool verbose) {
  intx max_value = max_intx;
#if defined(SPARC)
  max_value = 1;
#elif defined(X86)
  max_value = 3;
#endif
  if (value < 0 || value > max_value) {
    JVMFlag::printError(verbose,
                        "AllocatePrefetchInstr (" INTX_FORMAT ") must be "
                        "between 0 and " INTX_FORMAT "\n", value, max_value);
    return JVMFlag::VIOLATES_CONSTRAINT;
  }

  return JVMFlag::SUCCESS;
}

JVMFlag::Error CompileThresholdConstraintFunc(intx value, bool verbose) {
  if (value < 0 || value > INT_MAX >> InvocationCounter::count_shift) {
    JVMFlag::printError(verbose,
                        "CompileThreshold (" INTX_FORMAT ") "
                        "must be between 0 and %d\n",
                        value,
                        INT_MAX >> InvocationCounter::count_shift);
    return JVMFlag::VIOLATES_CONSTRAINT;
  }

  return JVMFlag::SUCCESS;
}

JVMFlag::Error OnStackReplacePercentageConstraintFunc(intx value, bool verbose) {
  int backward_branch_limit;
  if (ProfileInterpreter) {
    if (OnStackReplacePercentage < InterpreterProfilePercentage) {
      JVMFlag::printError(verbose,
                          "OnStackReplacePercentage (" INTX_FORMAT ") must be "
                          "larger than InterpreterProfilePercentage (" INTX_FORMAT ")\n",
                          OnStackReplacePercentage, InterpreterProfilePercentage);
      return JVMFlag::VIOLATES_CONSTRAINT;
    }

    backward_branch_limit = ((CompileThreshold * (OnStackReplacePercentage - InterpreterProfilePercentage)) / 100)
                            << InvocationCounter::count_shift;

    if (backward_branch_limit < 0) {
      JVMFlag::printError(verbose,
                          "CompileThreshold * (InterpreterProfilePercentage - OnStackReplacePercentage) / 100 = "
                          INTX_FORMAT " "
                          "must be between 0 and " INTX_FORMAT ", try changing "
                          "CompileThreshold, InterpreterProfilePercentage, and/or OnStackReplacePercentage\n",
                          (CompileThreshold * (OnStackReplacePercentage - InterpreterProfilePercentage)) / 100,
                          INT_MAX >> InvocationCounter::count_shift);
      return JVMFlag::VIOLATES_CONSTRAINT;
    }
  } else {
    if (OnStackReplacePercentage < 0 ) {
      JVMFlag::printError(verbose,
                          "OnStackReplacePercentage (" INTX_FORMAT ") must be "
                          "non-negative\n", OnStackReplacePercentage);
      return JVMFlag::VIOLATES_CONSTRAINT;
    }

    backward_branch_limit = ((CompileThreshold * OnStackReplacePercentage) / 100)
                            << InvocationCounter::count_shift;

    if (backward_branch_limit < 0) {
      JVMFlag::printError(verbose,
                          "CompileThreshold * OnStackReplacePercentage / 100 = " INTX_FORMAT " "
                          "must be between 0 and " INTX_FORMAT ", try changing "
                          "CompileThreshold and/or OnStackReplacePercentage\n",
                          (CompileThreshold * OnStackReplacePercentage) / 100,
                          INT_MAX >> InvocationCounter::count_shift);
      return JVMFlag::VIOLATES_CONSTRAINT;
    }
  }
  return JVMFlag::SUCCESS;
}

JVMFlag::Error CodeCacheSegmentSizeConstraintFunc(uintx value, bool verbose) {
  if (CodeCacheSegmentSize < (uintx)CodeEntryAlignment) {
    JVMFlag::printError(verbose,
                        "CodeCacheSegmentSize  (" UINTX_FORMAT ") must be "
                        "larger than or equal to CodeEntryAlignment (" INTX_FORMAT ") "
                        "to align entry points\n",
                        CodeCacheSegmentSize, CodeEntryAlignment);
    return JVMFlag::VIOLATES_CONSTRAINT;
  }

  if (CodeCacheSegmentSize < sizeof(jdouble)) {
    JVMFlag::printError(verbose,
                        "CodeCacheSegmentSize  (" UINTX_FORMAT ") must be "
                        "at least " SIZE_FORMAT " to align constants\n",
                        CodeCacheSegmentSize, sizeof(jdouble));
    return JVMFlag::VIOLATES_CONSTRAINT;
  }

#ifdef COMPILER2
  if (CodeCacheSegmentSize < (uintx)OptoLoopAlignment) {
    JVMFlag::printError(verbose,
                        "CodeCacheSegmentSize  (" UINTX_FORMAT ") must be "
                        "larger than or equal to OptoLoopAlignment (" INTX_FORMAT ") "
                        "to align inner loops\n",
                        CodeCacheSegmentSize, OptoLoopAlignment);
    return JVMFlag::VIOLATES_CONSTRAINT;
  }
#endif

  return JVMFlag::SUCCESS;
}

JVMFlag::Error CompilerThreadPriorityConstraintFunc(intx value, bool verbose) {
#ifdef SOLARIS
  if ((value < MinimumPriority || value > MaximumPriority) &&
      (value != -1) && (value != -FXCriticalPriority)) {
    JVMFlag::printError(verbose,
                        "CompileThreadPriority (" INTX_FORMAT ") must be "
                        "between %d and %d inclusively or -1 (means no change) "
                        "or %d (special value for critical thread class/priority)\n",
                        value, MinimumPriority, MaximumPriority, -FXCriticalPriority);
    return JVMFlag::VIOLATES_CONSTRAINT;
  }
#endif

  return JVMFlag::SUCCESS;
}

JVMFlag::Error CodeEntryAlignmentConstraintFunc(intx value, bool verbose) {
#ifdef SPARC
  if (CodeEntryAlignment % relocInfo::addr_unit() != 0) {
    JVMFlag::printError(verbose,
                        "CodeEntryAlignment (" INTX_FORMAT ") must be "
                        "multiple of NOP size\n", CodeEntryAlignment);
    return JVMFlag::VIOLATES_CONSTRAINT;
  }
#endif

  if (!is_power_of_2(value)) {
    JVMFlag::printError(verbose,
                        "CodeEntryAlignment (" INTX_FORMAT ") must be "
                        "a power of two\n", CodeEntryAlignment);
    return JVMFlag::VIOLATES_CONSTRAINT;
  }

  if (CodeEntryAlignment < 16) {
      JVMFlag::printError(verbose,
                          "CodeEntryAlignment (" INTX_FORMAT ") must be "
                          "greater than or equal to %d\n",
                          CodeEntryAlignment, 16);
      return JVMFlag::VIOLATES_CONSTRAINT;
  }

  return JVMFlag::SUCCESS;
}

JVMFlag::Error OptoLoopAlignmentConstraintFunc(intx value, bool verbose) {
  if (!is_power_of_2(value)) {
    JVMFlag::printError(verbose,
                        "OptoLoopAlignment (" INTX_FORMAT ") "
                        "must be a power of two\n",
                        value);
    return JVMFlag::VIOLATES_CONSTRAINT;
  }

  // Relevant on ppc, s390, sparc. Will be optimized where
  // addr_unit() == 1.
  if (OptoLoopAlignment % relocInfo::addr_unit() != 0) {
    JVMFlag::printError(verbose,
                        "OptoLoopAlignment (" INTX_FORMAT ") must be "
                        "multiple of NOP size (%d)\n",
                        value, relocInfo::addr_unit());
    return JVMFlag::VIOLATES_CONSTRAINT;
  }

  return JVMFlag::SUCCESS;
}

JVMFlag::Error ArraycopyDstPrefetchDistanceConstraintFunc(uintx value, bool verbose) {
  if (value >= 4032) {
    JVMFlag::printError(verbose,
                        "ArraycopyDstPrefetchDistance (" UINTX_FORMAT ") must be"
                        "between 0 and 4031\n", value);
    return JVMFlag::VIOLATES_CONSTRAINT;
  }

  return JVMFlag::SUCCESS;
}

JVMFlag::Error ArraycopySrcPrefetchDistanceConstraintFunc(uintx value, bool verbose) {
  if (value >= 4032) {
    JVMFlag::printError(verbose,
                        "ArraycopySrcPrefetchDistance (" UINTX_FORMAT ") must be"
                        "between 0 and 4031\n", value);
    return JVMFlag::VIOLATES_CONSTRAINT;
  }

  return JVMFlag::SUCCESS;
}

JVMFlag::Error TypeProfileLevelConstraintFunc(uintx value, bool verbose) {
  for (int i = 0; i < 3; i++) {
    if (value % 10 > 2) {
      JVMFlag::printError(verbose,
                          "Invalid value (" UINTX_FORMAT ") "
                          "in TypeProfileLevel at position %d\n", value, i);
      return JVMFlag::VIOLATES_CONSTRAINT;
    }
    value = value / 10;
  }

  return JVMFlag::SUCCESS;
}

JVMFlag::Error InitArrayShortSizeConstraintFunc(intx value, bool verbose) {
  if (value % BytesPerLong != 0) {
    return JVMFlag::VIOLATES_CONSTRAINT;
  } else {
    return JVMFlag::SUCCESS;
  }
}

#ifdef COMPILER2
JVMFlag::Error InteriorEntryAlignmentConstraintFunc(intx value, bool verbose) {
  if (InteriorEntryAlignment > CodeEntryAlignment) {
    JVMFlag::printError(verbose,
                       "InteriorEntryAlignment (" INTX_FORMAT ") must be "
                       "less than or equal to CodeEntryAlignment (" INTX_FORMAT ")\n",
                       InteriorEntryAlignment, CodeEntryAlignment);
    return JVMFlag::VIOLATES_CONSTRAINT;
  }

#ifdef SPARC
  if (InteriorEntryAlignment % relocInfo::addr_unit() != 0) {
    JVMFlag::printError(verbose,
                        "InteriorEntryAlignment (" INTX_FORMAT ") must be "
                        "multiple of NOP size\n");
    return JVMFlag::VIOLATES_CONSTRAINT;
  }
#endif

  if (!is_power_of_2(value)) {
     JVMFlag::printError(verbose,
                         "InteriorEntryAlignment (" INTX_FORMAT ") must be "
                         "a power of two\n", InteriorEntryAlignment);
     return JVMFlag::VIOLATES_CONSTRAINT;
   }

  int minimum_alignment = 16;
#if defined(SPARC) || (defined(X86) && !defined(AMD64))
  minimum_alignment = 4;
#elif defined(S390)
  minimum_alignment = 2;
#endif

  if (InteriorEntryAlignment < minimum_alignment) {
    JVMFlag::printError(verbose,
                        "InteriorEntryAlignment (" INTX_FORMAT ") must be "
                        "greater than or equal to %d\n",
                        InteriorEntryAlignment, minimum_alignment);
    return JVMFlag::VIOLATES_CONSTRAINT;
  }

  return JVMFlag::SUCCESS;
}

JVMFlag::Error NodeLimitFudgeFactorConstraintFunc(intx value, bool verbose) {
  if (value < MaxNodeLimit * 2 / 100 || value > MaxNodeLimit * 40 / 100) {
    JVMFlag::printError(verbose,
                        "NodeLimitFudgeFactor must be between 2%% and 40%% "
                        "of MaxNodeLimit (" INTX_FORMAT ")\n",
                        MaxNodeLimit);
    return JVMFlag::VIOLATES_CONSTRAINT;
  }

  return JVMFlag::SUCCESS;
}
#endif // COMPILER2

JVMFlag::Error RTMTotalCountIncrRateConstraintFunc(int value, bool verbose) {
#if INCLUDE_RTM_OPT
  if (UseRTMLocking && !is_power_of_2(RTMTotalCountIncrRate)) {
    JVMFlag::printError(verbose,
                        "RTMTotalCountIncrRate (" INTX_FORMAT
                        ") must be a power of 2, resetting it to 64\n",
                        RTMTotalCountIncrRate);
    FLAG_SET_DEFAULT(RTMTotalCountIncrRate, 64);
  }
#endif

  return JVMFlag::SUCCESS;
}
