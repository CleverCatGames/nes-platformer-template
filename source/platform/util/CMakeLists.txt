cmake_minimum_required(VERSION 3.11)

add_library(platform_util_lib
    call.s
    decsp3.s
    incsp2.s
    makebool.s
    popa.s
    pusha.s
    pushax.s
    zeropage.s
)

