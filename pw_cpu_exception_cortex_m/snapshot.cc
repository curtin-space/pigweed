// Copyright 2021 The Pigweed Authors
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.

#define PW_LOG_LEVEL PW_CPU_EXCEPTION_CORTEX_M_LOG_LEVEL

#include "pw_cpu_exception_cortex_m/snapshot.h"

#include "pw_cpu_exception_cortex_m/proto_dump.h"
#include "pw_cpu_exception_cortex_m_private/config.h"
#include "pw_cpu_exception_cortex_m_private/cortex_m_constants.h"
#include "pw_cpu_exception_cortex_m_protos/cpu_state.pwpb.h"
#include "pw_log/log.h"
#include "pw_protobuf/encoder.h"
#include "pw_status/status.h"
#include "pw_thread/snapshot.h"
#include "pw_thread_protos/thread.pwpb.h"

namespace pw::cpu_exception_cortex_m {
namespace {

constexpr char kMainStackHandlerModeName[] = "Main Stack (Handler Mode)";
constexpr char kMainStackThreadModeName[] = "Main Stack (Thread Mode)";

enum class ProcessorMode {
  kHandlerMode,
  kThreadMode,
};

Status CaptureMainStack(
    ProcessorMode mode,
    uintptr_t stack_low_addr,
    uintptr_t stack_high_addr,
    uintptr_t stack_pointer,
    thread::SnapshotThreadInfo::StreamEncoder& snapshot_encoder,
    thread::ProcessThreadStackCallback& thread_stack_callback) {
  thread::Thread::StreamEncoder encoder = snapshot_encoder.GetThreadsEncoder();

  const char* thread_name;
  thread::ThreadState::Enum thread_state;
  if (mode == ProcessorMode::kHandlerMode) {
    thread_name = kMainStackHandlerModeName;
    PW_LOG_DEBUG("Capturing thread info for Main Stack (Handler Mode)");
    thread_state = thread::ThreadState::Enum::INTERRUPT_HANDLER;
    PW_LOG_DEBUG("Thread state: INTERRUPT_HANDLER");
  } else {  // mode == ProcessorMode::kThreadMode
    thread_name = kMainStackHandlerModeName;
    PW_LOG_DEBUG("Capturing thread info for Main Stack (Thread Mode)");
    thread_state = thread::ThreadState::Enum::RUNNING;
    PW_LOG_DEBUG("Thread state: RUNNING");
  }
  encoder.WriteState(thread_state);
  encoder.WriteName(std::as_bytes(std::span(std::string_view(thread_name))));

  const thread::StackContext thread_ctx = {
      .thread_name = thread_name,
      .stack_low_addr = stack_low_addr,
      .stack_high_addr = stack_high_addr,
      .stack_pointer = stack_pointer,
      .stack_pointer_est_peak = std::nullopt,
  };
  return thread::SnapshotStack(thread_ctx, encoder, thread_stack_callback);
}

}  // namespace

Status SnapshotCpuState(
    const pw_cpu_exception_State& cpu_state,
    cpu_exception::cortex_m::SnapshotCpuState::StreamEncoder&
        snapshot_encoder) {
  cpu_exception::LogCpuState(cpu_state);
  {
    cpu_exception::cortex_m::ArmV7mCpuState::StreamEncoder cpu_state_encoder =
        snapshot_encoder.GetArmv7mCpuStateEncoder();
    pw::cpu_exception::DumpCpuStateProto(cpu_state_encoder, cpu_state);
  }
  return snapshot_encoder.status();
}

Status SnapshotMainStackThread(
    uintptr_t stack_low_addr,
    uintptr_t stack_high_addr,
    thread::SnapshotThreadInfo::StreamEncoder& encoder,
    thread::ProcessThreadStackCallback& thread_stack_callback) {
  uintptr_t stack_pointer;
  asm volatile("mrs %0, msp\n" : "=r"(stack_pointer));

  // First check if we're in Handler mode, AKA handling exceptions/interrupts.
  //
  // Handler mode vs thread mode can be determined via IPSR, bits 8:0 of xPSR.
  // In thread mode the value is 0, in handler mode the value is non-zero.
  uint32_t xpsr;
  asm volatile("mrs %0, xpsr\n" : "=r"(xpsr));
  if ((xpsr & cpu_exception::kXpsrIpsrMask) != 0) {
    return CaptureMainStack(ProcessorMode::kHandlerMode,
                            stack_low_addr,
                            stack_high_addr,
                            stack_pointer,
                            encoder,
                            thread_stack_callback);
  }

  // It looks like we're in Thread mode which means we need to check whether
  // or not we are executing off the main stack currently.
  //
  // See ARMv7-M Architecture Reference Manual Section B1.4.4 for the control
  // register values, in particular the SPSEL bit while in Thread mode which
  // is 0 while running off the main stack and 1 while running off the proces
  // stack.
  uint32_t control;
  asm volatile("mrs %0, control\n" : "=r"(control));
  if ((control & cpu_exception::kControlThreadModeStackMask) != 0) {
    return OkStatus();  // Main stack is not currently active.
  }

  // We're running off the main stack in Thread mode.
  return CaptureMainStack(ProcessorMode::kThreadMode,
                          stack_low_addr,
                          stack_high_addr,
                          stack_pointer,
                          encoder,
                          thread_stack_callback);
}

Status SnapshotMainStackThread(
    const pw_cpu_exception_State& cpu_state,
    uintptr_t stack_low_addr,
    uintptr_t stack_high_addr,
    thread::SnapshotThreadInfo::StreamEncoder& encoder,
    thread::ProcessThreadStackCallback& thread_stack_callback) {
  const uint32_t exc_return = cpu_state.extended.exc_return;

  // See ARMv7-M Architecture Reference Manual Section B1.5.8 for the exception
  // return values, in particular bits 0:3.
  // Bits 0:3 of EXC_RETURN:
  // 0b0001 - 0x1 Handler mode Main
  // 0b1001 - 0x9 Thread mode Main
  // 0b1101 - 0xD Thread mode Process

  // First check whether the CPU state shows the main stack was active.
  if ((exc_return & cpu_exception::kExcReturnStackMask) != 0) {
    return OkStatus();  // Main stack is not currently active.
  }
  const uintptr_t stack_pointer = cpu_state.extended.msp;

  // Second, check if we're in Handler mode, AKA handling exceptions/interrupts.
  const ProcessorMode mode =
      ((exc_return & cpu_exception::kExcReturnModeMask) == 0)
          ? ProcessorMode::kHandlerMode
          : ProcessorMode::kThreadMode;

  return CaptureMainStack(mode,
                          stack_low_addr,
                          stack_high_addr,
                          stack_pointer,
                          encoder,
                          thread_stack_callback);
}

}  // namespace pw::cpu_exception_cortex_m
