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
							"/usr/include/PCSC"
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
						]
					}
				]
			]
    }
  ]
}
