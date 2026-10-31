# Sanitizer 开关：默认关闭，CI 与本地检查脚本显式打开。
# 打开后能发现越界、悬垂指针、未定义行为与数据竞争（TSan 需单独开启）。

option(CLAB_ENABLE_SANITIZERS "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

function(clab_enable_sanitizers target)
    if(NOT CLAB_ENABLE_SANITIZERS)
        return()
    endif()
    if(MSVC)
        # MSVC 不支持 -fsanitize，使用 /fsanitize=address（需要 VS 2019+）
        target_compile_options(${target} PRIVATE /fsanitize=address)
        return()
    endif()
    target_compile_options(${target} PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -fno-sanitize-recover=all
    )
    target_link_options(${target} PRIVATE -fsanitize=address,undefined)
endfunction()
