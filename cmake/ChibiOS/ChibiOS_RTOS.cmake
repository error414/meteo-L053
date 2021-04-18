FOREACH (FAMILY F0 L0 L4)
    SET (CHIBIOS_SOURCES_${CHIBIOS_KERNEL}_${FAMILY}
        os/common/startup/ARMCMx/compilers/GCC/crt0_v6m.S
        os/common/ports/ARMv6-M/chcore.c
        os/common/ports/ARMv6-M/compilers/GCC/chcoreasm.S
    )
ENDFOREACH()

FOREACH (FAMILY F1 F2 F3 F4 F7 H7)
    SET (CHIBIOS_SOURCES_${CHIBIOS_KERNEL}_${FAMILY}
        os/common/startup/ARMCMx/compilers/GCC/crt0_v7m.S
        os/common/ports/ARMCMx/compilers/GCC/chcoreasm_v7m.S
        os/common/ports/ARMCMx/chcore.c
        os/common/ports/ARMCMx/chcore_v7m.c

    )
ENDFOREACH()

FOREACH (FAMILY F0 L0 L4)
    SET (CHIBIOS_INCLUDES_${CHIBIOS_KERNEL}_${FAMILY}
            os/common/startup/ARMCMx/devices/STM32${FAMILY}xx
            os/common/ext/ST/STM32${FAMILY}xx
            os/common/oslib/include
            os/common/ports/ARMv6-M
            os/common/ports/ARMCMx/compilers/GCC
            )
ENDFOREACH()

FOREACH (FAMILY F1 F2 F3 F4 F7 H7)
    SET (CHIBIOS_INCLUDES_${CHIBIOS_KERNEL}_${FAMILY}
        os/common/startup/ARMCMx/devices/STM32${FAMILY}xx
        os/common/ext/ST/STM32${FAMILY}xx
        os/common/oslib/include
        os/common/ports/ARMCMx
        os/common/ports/ARMCMx/compilers/GCC
    )
ENDFOREACH()

SET (CHIBIOS_SOURCES_${CHIBIOS_KERNEL}
    os/common/startup/ARMCMx/compilers/GCC/crt1.c
    os/common/startup/ARMCMx/compilers/GCC/vectors.S
)

SET (CHIBIOS_INCLUDES_${CHIBIOS_KERNEL}
    os/license
    os/common/portability/GCC
    os/common/startup/ARMCMx/compilers/GCC
    os/common/ext/ARM/CMSIS/Core/Include
)

IF (${CHIBIOS_DEBUG_ENABLE_SHELL})
    LIST(APPEND CHIBIOS_INCLUDES_${CHIBIOS_KERNEL} os/various/shell)
    LIST(APPEND CHIBIOS_SOURCES_${CHIBIOS_KERNEL} os/various/shell/shell.c)
ENDIF()

IF (${CHIBIOS_DEBUG_ENABLE_SHELL_CMD})
    LIST(APPEND CHIBIOS_SOURCES_${CHIBIOS_KERNEL} os/various/shell/shell_cmd.c)
ENDIF()

SET (CHIBIOS_SOURCES_${CHIBIOS_KERNEL}_MAILBOXES  os/oslib/src/chmboxes.c)
SET (CHIBIOS_SOURCES_${CHIBIOS_KERNEL}_MEMPOOLS   os/oslib/src/chmempools.c)
SET (CHIBIOS_SOURCES_${CHIBIOS_KERNEL}_FACTORY    os/oslib/src/chfactory.c)
SET (CHIBIOS_SOURCES_${CHIBIOS_KERNEL}_FACTORY    os/oslib/src/chobjcaches.c)
SET (CHIBIOS_SOURCES_${CHIBIOS_KERNEL}_FACTORY    os/oslib/src/chpipes.c)
SET (CHIBIOS_SOURCES_${CHIBIOS_KERNEL}_MEMCORE    os/oslib/src/chmemcore.c)
LIST (APPEND CHIBIOS_SOURCES_${CHIBIOS_KERNEL}_MEMCORE  os/various/syscalls.c)

SET (CHIBIOS_SOURCES_rt_TM          os/rt/src/chtm.c)
SET (CHIBIOS_SOURCES_rt_REGISTRY    os/rt/src/chregistry.c)
SET (CHIBIOS_SOURCES_rt_SEMAPHORES  os/rt/src/chsem.c)
SET (CHIBIOS_SOURCES_rt_MUTEXES     os/rt/src/chmtx.c)
SET (CHIBIOS_SOURCES_rt_CONDVARS    os/rt/src/chcond.c)
SET (CHIBIOS_SOURCES_rt_EVENTS      os/rt/src/chevents.c)
SET (CHIBIOS_SOURCES_rt_MESSAGES    os/rt/src/chmsg.c)
SET (CHIBIOS_SOURCES_rt_DYNAMIC     os/rt/src/chdynamic.c)

LIST (APPEND CHIBIOS_SOURCES_nil     os/nil/src/ch.c)
LIST (APPEND CHIBIOS_INCLUDES_nil    os/nil/include)

LIST (APPEND CHIBIOS_SOURCES_rt
    os/rt/src/chsys.c
    os/rt/src/chvt.c
    os/rt/src/chschd.c
    os/rt/src/chthreads.c
    os/oslib/src/chmemheaps.c
    os/oslib/src/chobjcaches.c
    os/oslib/src/chfactory.c
)

LIST (APPEND CHIBIOS_INCLUDES_rt  os/rt/include)
LIST (APPEND CHIBIOS_INCLUDES_rt  os/oslib/include)

IF (CHIBIOS_SOURCES_${CHIBIOS_KERNEL}_${STM32_FAMILY})
    LIST(APPEND CHIBIOS_SOURCES_${CHIBIOS_KERNEL} ${CHIBIOS_SOURCES_${CHIBIOS_KERNEL}_${STM32_FAMILY}})
ENDIF()

IF (CHIBIOS_INCLUDES_${CHIBIOS_KERNEL}_${STM32_FAMILY})
    LIST(APPEND CHIBIOS_INCLUDES_${CHIBIOS_KERNEL} ${CHIBIOS_INCLUDES_${CHIBIOS_KERNEL}_${STM32_FAMILY}})
ENDIF()

FOREACH (COMP ${CHIBIOS_RTOS_COMPONENTS})
    IF (CHIBIOS_SOURCES_${CHIBIOS_KERNEL}_${COMP})
        LIST(APPEND CHIBIOS_SOURCES_${CHIBIOS_KERNEL} ${CHIBIOS_SOURCES_${CHIBIOS_KERNEL}_${COMP}})
    ENDIF()
    IF (CHIBIOS_INCLUDES_${CHIBIOS_KERNEL}_${COMP})
        LIST(APPEND CHIBIOS_INCLUDES_${CHIBIOS_KERNEL} ${CHIBIOS_INCLUDES_${CHIBIOS_KERNEL}_${COMP}})
    ENDIF()

    IF (CHIBIOS_SOURCES_${CHIBIOS_KERNEL}_${COMP}_${STM32_FAMILY})
        LIST(APPEND CHIBIOS_SOURCES_${CHIBIOS_KERNEL} ${CHIBIOS_SOURCES_${CHIBIOS_KERNEL}_${COMP}_${STM32_FAMILY}})
    ENDIF()
    IF (CHIBIOS_INCLUDES_${CHIBIOS_KERNEL}_${COMP}_${STM32_FAMILY})
        LIST(APPEND CHIBIOS_INCLUDES_${CHIBIOS_KERNEL} ${CHIBIOS_INCLUDES_${CHIBIOS_KERNEL}_${COMP}_${STM32_FAMILY}})
    ENDIF()
ENDFOREACH()
