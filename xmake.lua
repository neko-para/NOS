toolchain("i686")
    set_kind("standalone")

    set_toolset("cc", "i686-elf-gcc")
    set_toolset("cxx", "i686-elf-g++")
    set_toolset("ld", "i686-elf-ld")
    set_toolset("sh", "i686-elf-gcc")
    set_toolset("ar", "i686-elf-ar")
    set_toolset("as", "i686-elf-gcc")
    set_toolset("strip", "i686-elf-strip")
    
    on_load(function (toolchain)
        toolchain:add("cxxflags", "-ffreestanding")
        toolchain:add("cxxflags", "-Wall")
        toolchain:add("cxxflags", "-Wextra")
        toolchain:add("cxxflags", "-fno-rtti")
        toolchain:add("cxxflags", "-fno-exceptions")
        toolchain:add("cxxflags", "-m32")
        toolchain:add("asflags", "-m32")
        toolchain:add("ldflags", "-T kernel/link.ld")
        toolchain:add("ldflags", "-L/home/nekosu/osdev/inst/lib/gcc/i686-elf/11.2.0")
        toolchain:add("ldflags", "-lgcc")
    end)

target("kernel")
    set_kind("binary")
    set_toolchains("i686")
    add_files("kernel/boot/*.s")
    add_files("kernel/boot/*.cpp")
    add_files("kernel/*.cpp")
    add_includedirs("$(projectdir)")
    after_build(function (target)
        import("core.project.depend")
        depend.on_changed(function ()
            os.cp(target:targetfile(), "$(projectdir)/disk/boot/kernel.bin")
            os.vrunv("make", {"install"})
        end, {files = target:targetfile()})
    end)

task("run")
    on_run(function (target)
        os.vrunv("make", {"run"})
    end)
    set_menu {
        usage = "xmake run",
        description = "Run by qemu!",
        options = {}
    } 