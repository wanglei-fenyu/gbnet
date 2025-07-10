include_guard()

function(collectHeadFiles resultListVar searchDirsVar)
    set(files_ "")
    foreach(dir_ IN LISTS ${searchDirsVar})
        file(GLOB_RECURSE tempList_ "${dir_}/*.h" "${dir_}/*.hpp" "${dir_}/*.inl" "${dir_}/*.inc")
        list(APPEND files_ ${tempList_})
    endforeach()

    set(${resultListVar} ${files_} PARENT_SCOPE)
endfunction()

function(collectSrcFiles resultListVar searchDirsVar)
    set(files_ "")
    foreach(dir_ IN LISTS ${searchDirsVar})
        file(GLOB_RECURSE tempList_ "${dir_}/*.cpp" "${dir_}/*.c" "${dir_}/*.cc")
        list(APPEND files_ ${tempList_})
    endforeach()
    set(${resultListVar} ${files_} PARENT_SCOPE)
endfunction()

function(collectAllSrcFiles resultListVar searchDirsVar)
    set(files_ "")
    foreach(dir_ IN LISTS ${searchDirsVar})
        file(GLOB_RECURSE tempList_ "${dir_}/*.h" "${dir_}/*.hpp" "${dir_}/*.inl" "${dir_}/*.inc" "${dir_}/*.cpp" "${dir_}/*.c" "${dir_}/*.cc")
        list(APPEND files_ ${tempList_})
    endforeach()
    set(${resultListVar} ${files_} PARENT_SCOPE)
endfunction()

# 给 list 每个元素添加前缀
function(appendPrefixInList resultListVar inputListVar prefixStr)
    set(newList_ "")
    foreach(element_ IN LISTS ${inputListVar})
        list(APPEND newList_ "${prefixStr}${element_}")
    endforeach()
    set(${resultListVar} ${newList_} PARENT_SCOPE)
endfunction()

function(removePathsFromListByStrMatch fileListVar excludePathListVar)
    set(removeList_ "")
    foreach(filePath IN LISTS ${fileListVar})
        foreach(excludePath IN LISTS ${excludePathListVar})
            string(FIND ${filePath} ${excludePath} resultDir_)
            if(resultDir_ GREATER -1)
                # message(STATUS "resultDir_:${resultDir_} filePath:${filePath}")
                list(APPEND removeList_ ${filePath})
                break()
            endif()
        endforeach()
    endforeach()
    
    foreach(removeElement_ IN LISTS removeList_)
        # message(STATUS "remove: ${removeDir_}")
        list(REMOVE_ITEM ${fileListVar} ${removeElement_})
    endforeach()

    set(${fileListVar} ${${fileListVar}} PARENT_SCOPE)
endfunction()

function(appendSuffixInList result_var input_list suffix)
    # 初始化结果列表
    set(result "")
    foreach(item ${input_list})
        message(STATUS "appendSuffixInList input : ${item}")
        list(APPEND result "${item}${suffix}")
    endforeach()
    # 使用 LISTS 关键字将结果列表传递给父作用域
    set(${result_var} ${result} PARENT_SCOPE)
endfunction()



function(find_multiple_libraries result_list path lib_names)
    set(result "")

    foreach(lib_name ${lib_names})

        # 查找库
        find_library(local_found_lib NAMES ${lib_name} PATHS ${path})
        
        # 打印调试信息
        message(STATUS "Attempting to find ${lib_name} in ${path}")
        message(STATUS "Found library path: ${local_found_lib}")

        if(local_found_lib)
            # 如果找到，将库路径追加到结果列表
            list(APPEND result ${local_found_lib})
            message(STATUS "Successfully found ${lib_name}: ${local_found_lib}")
        else()
            message(WARNING "Could not find ${lib_name}")
        endif()
		unset(local_found_lib CACHE)
    endforeach()

    # 将结果列表设置到父作用域
    set(${result_list} ${result} PARENT_SCOPE)
endfunction()
