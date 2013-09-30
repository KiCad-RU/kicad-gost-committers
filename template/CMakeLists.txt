install(FILES kicad.pro fp_global_table fp_global_table.csv 
	gost_landscape.kicad_wks gost_portrait.kicad_wks pagelayout_default.kicad_wks pagelayout_logo.kicad_wks
        DESTINATION ${KICAD_TEMPLATE}
        COMPONENT resources)
