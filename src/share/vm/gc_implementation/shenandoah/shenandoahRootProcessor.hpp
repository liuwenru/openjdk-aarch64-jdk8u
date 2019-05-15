/*
 * Copyright (c) 2015, 2018, Red Hat, Inc. All rights reserved.
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

#ifndef SHARE_VM_GC_SHENANDOAH_SHENANDOAHROOTPROCESSOR_HPP
#define SHARE_VM_GC_SHENANDOAH_SHENANDOAHROOTPROCESSOR_HPP

#include "code/codeCache.hpp"
#include "gc_implementation/shenandoah/shenandoahHeap.hpp"
#include "gc_implementation/shenandoah/shenandoahCollectorPolicy.hpp"
#include "gc_implementation/shenandoah/shenandoahCodeRoots.hpp"
#include "gc_implementation/shenandoah/shenandoahPhaseTimings.hpp"
#include "gc_implementation/shenandoah/shenandoahSynchronizerIterator.hpp"
#include "gc_implementation/shenandoah/shenandoahUtils.hpp"
#include "memory/allocation.hpp"
#include "utilities/workgroup.hpp"

class ParallelCLDRootIterator VALUE_OBJ_CLASS_SPEC {
public:
  ParallelCLDRootIterator();
  void root_cld_do(CLDClosure* strong, CLDClosure* weak);
};

enum Shenandoah_process_roots_tasks {
  SHENANDOAH_RP_PS_Universe_oops_do,
  SHENANDOAH_RP_PS_JNIHandles_oops_do,
  SHENANDOAH_RP_PS_JNIHandles_weak_oops_do,
  SHENANDOAH_RP_PS_FlatProfiler_oops_do,
  SHENANDOAH_RP_PS_Management_oops_do,
  SHENANDOAH_RP_PS_SystemDictionary_oops_do,
  SHENANDOAH_RP_PS_jvmti_oops_do,
  // Leave this one last.
  SHENANDOAH_RP_PS_NumElements
};

class ShenandoahRootProcessor : public StackObj {
  SubTasksDone* _process_strong_tasks;
  SharedHeap::StrongRootsScope _srs;
  ShenandoahPhaseTimings::Phase _phase;
  ShenandoahGCWorkerPhase       _worker_phase;
  ParallelCLDRootIterator   _cld_iterator;
  ShenandoahAllCodeRootsIterator _coderoots_all_iterator;
  ShenandoahSynchronizerIterator _om_iterator;

  void process_java_roots(OopClosure* scan_non_heap_roots,
                          CLDClosure* scan_strong_clds,
                          CLDClosure* scan_weak_clds,
                          CodeBlobClosure* scan_strong_code,
                          ThreadClosure* thread_cl,
                          uint worker_i);

  void process_vm_roots(OopClosure* scan_non_heap_roots,
                        OopClosure* weak_jni_roots,
                        BoolObjectClosure* is_alive,
                        uint worker_i);

public:
  ShenandoahRootProcessor(ShenandoahHeap* heap, uint n_workers,
                          ShenandoahPhaseTimings::Phase phase);
  ~ShenandoahRootProcessor();

  // Apply oops, clds and blobs to all strongly reachable roots in the system.
  // Optionally, apply class loader closure to weak clds, depending on class unloading
  // for the particular GC cycles.
  void process_strong_roots(OopClosure* oops,
                            CLDClosure* clds,
                            CodeBlobClosure* blobs,
                            ThreadClosure* thread_cl,
                            uint worker_id);

  // Apply oops, clds and blobs to strongly reachable roots in the system
  void process_all_roots(OopClosure* oops,
                         CLDClosure* clds,
                         CodeBlobClosure* blobs,
                         ThreadClosure* thread_cl,
                         uint worker_id);

  // Apply oops, clds and blobs to strongly and weakly reachable roots in the system
  template <typename IsAlive>
  void update_all_roots(OopClosure* oops,
                        CLDClosure* clds,
                        CodeBlobClosure* blobs,
                        ThreadClosure* thread_cl,
                        uint worker_id);


  // Number of worker threads used by the root processor.
  uint n_workers() const;
};

class ShenandoahRootEvacuator : public StackObj {
  SubTasksDone* _evacuation_tasks;
  SharedHeap::StrongRootsScope _srs;
  ShenandoahPhaseTimings::Phase _phase;
  ParallelCLDRootIterator   _cld_iterator;
  ShenandoahCsetCodeRootsIterator _coderoots_cset_iterator;
  ShenandoahSynchronizerIterator _om_iterator;

  enum Shenandoah_evacuate_roots_tasks {
    SHENANDOAH_EVAC_Universe_oops_do,
    SHENANDOAH_EVAC_JNIHandles_oops_do,
    SHENANDOAH_EVAC_JNIHandles_weak_oops_do,
    SHENANDOAH_EVAC_FlatProfiler_oops_do,
    SHENANDOAH_EVAC_Management_oops_do,
    SHENANDOAH_EVAC_SystemDictionary_oops_do,
    SHENANDOAH_EVAC_jvmti_oops_do,
    // Leave this one last.
    SHENANDOAH_EVAC_NumElements
  };
public:
  ShenandoahRootEvacuator(ShenandoahHeap* heap, uint n_workers,
                          ShenandoahPhaseTimings::Phase phase);
  ~ShenandoahRootEvacuator();

  void process_evacuate_roots(OopClosure* oops,
                              CodeBlobClosure* blobs,
                              uint worker_id);

  // Number of worker threads used by the root processor.
  uint n_workers() const;
};
#endif // SHARE_VM_GC_SHENANDOAH_SHENANDOAHROOTPROCESSOR_HPP
