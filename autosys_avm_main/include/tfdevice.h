/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef TENSORFLOW_LITE_DEVICE_H_
#define TENSORFLOW_LITE_DEVICE_H_

#include "tensorflow/lite/model.h"
#include "tensorflow/lite/string_type.h"

struct Settings {
  bool verbose = false;
  bool accel = false;
  TfLiteType input_type = kTfLiteFloat32;
  bool profiling = false;
  bool allow_fp16 = false;
  bool gl_backend = false;
  bool hexagon_delegate = false;
  bool xnnpack_delegate = false;
  int loop_count = 1;
  float input_mean = 127.5f;
  float input_std = 127.5f;
  std::string model_name = "./mobilenet_v1_1.0_224_quant.tflite";
  tflite::FlatBufferModel* model;
  std::string input_bmp_name = "./grace_hopper.bmp";
  std::string labels_file_name = "./labels.txt";
  int number_of_threads = 4;
  int number_of_results = 5;
  int max_profiling_buffer_entries = 1024;
  int number_of_warmup_runs = 2;
  int caching = false;
};


using TfLiteDelegatePtr = tflite::Interpreter::TfLiteDelegatePtr;
using ProvidedDelegateList = tflite::tools::ProvidedDelegateList;

class DelegateProviders {
    public:
    DelegateProviders() : delegate_list_util_(&params_) {
        delegate_list_util_.AddAllDelegateParams();
        //delegate_list_util_.AppendCmdlineFlags(flags_);

        // Remove the "help" flag to avoid printing "--help=false"
        params_.RemoveParam("help");
        //delegate_list_util_.RemoveCmdlineFlag(flags_, "help");
    }

    bool InitFromCmdlineArgs(int* argc, const char** argv);
    void MergeSettingsIntoParams(const Settings& s);
    std::vector<ProvidedDelegateList::ProvidedDelegate> CreateAllDelegates();

    
    private:
    // Contain delegate-related parameters that are initialized from command-line
    // flags.
    tflite::tools::ToolParams params_;

    // A helper to create TfLite delegates.
    ProvidedDelegateList delegate_list_util_;

    // Contains valid flags
    std::vector<tflite::Flag> flags_;
};


#endif  // TENSORFLOW_LITE_EXAMPLES_LABEL_IMAGE_LABEL_IMAGE_H_
