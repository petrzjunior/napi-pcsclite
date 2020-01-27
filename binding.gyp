{
  "targets": [
    {
      "target_name": "pcsclite",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ "pcsclite.cpp" ],
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
						]
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