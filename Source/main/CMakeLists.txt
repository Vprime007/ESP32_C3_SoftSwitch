idf_component_register(
    
SRCS            "main.c"
                "softSwitcher.c"

                "userInterface/buttonController.c"
                "userInterface/ledController.c"
                "userInterface/userInterface.c"

                "sensors/adcController.c"
                "sensors/electricalController.c"
                "sensors/temperatureCOntroller.c"

INCLUDE_DIRS    "../main"
                "userInterface"
                "sensors"
)
