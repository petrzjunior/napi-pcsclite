{
  "targets": [
    {
      "target_name": "pcsclite",
      "sources": [ "src/pcsclite.c", "src/pcscbinding.c" ],
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
