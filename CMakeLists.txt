idf_component_register(SRC_DIRS "src"
					INCLUDE_DIRS "include"
					PRIV_REQUIRES esp_driver_ledc
									esp_driver_gpio
				)

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++23)
