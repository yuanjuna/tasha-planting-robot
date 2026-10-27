import os

Import("env")

framework_dir = env.PioPlatform().get_package_dir("framework-stm32cubef4")
freertos_dir = os.path.join(
    framework_dir, "Middlewares", "Third_Party", "FreeRTOS", "Source"
)

env.Append(
    CPPPATH=[
        os.path.join(freertos_dir, "include"),
        os.path.join(freertos_dir, "portable", "GCC", "ARM_CM4F"),
    ],
    LINKFLAGS=[
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mfpu=fpv4-sp-d16",
        "-mfloat-abi=hard",
    ],
)

env.BuildSources(
    os.path.join("$BUILD_DIR", "FreeRTOS"),
    freertos_dir,
    src_filter=[
        "+<list.c>",
        "+<queue.c>",
        "+<stream_buffer.c>",
        "+<tasks.c>",
        "+<timers.c>",
        "+<event_groups.c>",
        "+<portable/GCC/ARM_CM4F/port.c>",
        "+<portable/MemMang/heap_4.c>",
    ],
)
