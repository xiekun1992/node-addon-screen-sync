{
  "targets": [
    {
      "target_name": "screen_sync",
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],
      "sources": [
        "src/screen_sync.cpp",
        "src/wrapper/screen_sync_wrapper.cpp",
        "src/wrapper/wrapper.cpp",
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "<(module_root_dir)/include/"
      ],
      "copies": [
        {
          "destination": "<(module_root_dir)/build/Release/",
          "files": [
            "<(module_root_dir)/bin/avcodec-58.dll",
            "<(module_root_dir)/bin/avdevice-58.dll",
            "<(module_root_dir)/bin/avfilter-7.dll",
            "<(module_root_dir)/bin/avformat-58.dll",
            "<(module_root_dir)/bin/avutil-56.dll",
            "<(module_root_dir)/bin/postproc-55.dll",
            "<(module_root_dir)/bin/swresample-3.dll",
            "<(module_root_dir)/bin/swscale-5.dll"
          ]
        }
      ],
      "link_settings": {
        "libraries": [
          "<(module_root_dir)/lib/avcodec.lib",
          "<(module_root_dir)/lib/avdevice.lib",
          "<(module_root_dir)/lib/avfilter.lib",
          "<(module_root_dir)/lib/avformat.lib",
          "<(module_root_dir)/lib/avutil.lib",
          "<(module_root_dir)/lib/postproc.lib",
          "<(module_root_dir)/lib/swresample.lib",
          "<(module_root_dir)/lib/swscale.lib"
        ]
      },
      "msvs_settings": {
        "VCLinkerTool": {
          'AdditionalLibraryDirectories': [
            '<(module_root_dir)/src/lib/'
          ]
        }
      },
      "dependencies": ["<!(node -p \"require('node-addon-api').gyp\")"],
      "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"]
    }
  ]
}