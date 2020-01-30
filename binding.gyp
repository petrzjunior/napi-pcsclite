{
  "targets": [
    {
      "target_name": "pcsclite",
      "cflags!": [ "-fno-exceptions"],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ "src/pcsclite.cpp", "src/pcscbinding.cpp" ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
      "conditions": [
				[
					"OS=='linux'",
					{
						"include_dirs": [
							"/usr/include/PCSC",
							"<!@(node -p \"require('node-addon-api').include\")"
						],
						"link_settings": {
							"libraries": [
								"-lpcsclite"
							],
							"library_dirs": [
								"/usr/lib"
							]
						}
					}
				],
				[
					"OS=='mac'",
					{
						"libraries": [
							"-framework",
							"PCSC"
						],
						"include_dirs": [
							"<!@(node -p \"require('node-addon-api').include\")"
						],
						"xcode_settings": {
        					"MACOSX_DEPLOYMENT_TARGET":"10.9"
      					}
					}
				],
				[
					"OS=='win'",
					{
						"libraries": [
							"-lWinSCard"
						],
						"include_dirs": [
							"<!@(node -p \"require('node-addon-api').include\")"
						]
					}
				]
			]
    }
  ]
}