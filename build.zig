
const std = @import("std");

pub fn build(b: *std.Build) !void {
    const enable_show_info = b.option(bool, "info", "TODO. (default 'true')") orelse true;
    // -Drelease=[bool]
    const optimize = b.standardOptimizeOption(.{ .preferred_optimize_mode = .ReleaseSmall });
    const target = b.resolveTargetQuery(.{
        .cpu_arch = .thumb,
        .os_tag = .freestanding,
        .abi = .eabihf,
        .cpu_model = .{ .explicit = &std.Target.arm.cpu.cortex_m23 },
    });

    const name = b.option([]const u8, "name", "The name of the object file. (default 'demo')") orelse "demo";
    const strip = b.option(bool, "strip", "Strip the debug info from the object file. (default false)") orelse true;

    const elf = b.addExecutable(.{
        .name = name,
        .target = target,
        .optimize = optimize,
        .link_libc = false,
        .linkage = .static,
        .single_threaded = true,
        .strip = strip,
        .error_tracing = false,
    });

    const path_to_startup = b.option([]const u8, "startup", "The relative path to the startup file. (default './startup.s')") orelse "./startup.s";
    elf.addAssemblyFile(b.path(path_to_startup));

    const path_to_linker_script = b.option([]const u8, "ld", "The relative path to the linker script file. (default './src/linker.ld')") orelse "./linker.ld";
    const entry_symbol_name = b.option([]const u8, "entry", "The entry symbol  of the program. (default 'Reset_Handler')") orelse "Reset_Handler";
    elf.linker_script = b.path(path_to_linker_script);
    elf.entry = .{ .symbol_name = entry_symbol_name };

    elf.addIncludePath(b.path("./gd32e23x"));
    elf.addIncludePath(b.path("./gd32e23x/core"));
    elf.addIncludePath(b.path("./gd32e23x/std/inc"));

    elf.defineCMacro("__SOFTFP__", null);

    if (b.option(bool, "rcu", "Use peripheral rcu. (default 'true')") orelse true) {
        elf.defineCMacro("GD32E23X_USE_RCU", null);
        elf.addCSourceFile(.{ .file = b.path("./gd32e23x/std/src/gd32e23x_rcu.c") });
    }
    if (b.option(bool, "fmc", "Use peripheral fmc. (default 'true')") orelse true) {
        elf.defineCMacro("GD32E23X_USE_FMC", null);
        elf.addCSourceFile(.{ .file = b.path("./gd32e23x/std/src/gd32e23x_fmc.c") });
    }
    if (b.option(bool, "misc", "Use peripheral misc. (default 'true')") orelse true) {
        elf.defineCMacro("GD32E23X_USE_MISC", null);
        elf.addCSourceFile(.{ .file = b.path("./gd32e23x/std/src/gd32e23x_misc.c") });
    }

    // TODO b.option to select the clock frequency
    elf.addCSourceFile(.{ .file = b.path("./gd32e23x/system_gd32e23x.c") });

    if (b.option(bool, "gpio", "Use peripheral gpio. (default 'false')") orelse false) {
        elf.defineCMacro("GD32E23X_USE_GPIO", null);
        elf.addCSourceFile(.{ .file = b.path("./gd32e23x/std/src/gd32e23x_gpio.c") });
    }
    if (b.option(bool, "usart", "Use peripheral usart. (default 'false')") orelse false) {
        elf.defineCMacro("GD32E23X_USE_USART", null);
        elf.addCSourceFile(.{ .file = b.path("./gd32e23x/std/src/gd32e23x_usart.c") });
    }
    if (b.option(bool, "dma", "Use peripheral dma. (default 'false')") orelse false) {
        elf.defineCMacro("GD32E23X_USE_DMA", null);
        elf.addCSourceFile(.{ .file = b.path("./gd32e23x/std/src/gd32e23x_dma.c") });
    }
    if (b.option(bool, "spi", "Use peripheral spi. (default 'false')") orelse false) {
        elf.defineCMacro("GD32E23X_USE_SPI", null);
        elf.addCSourceFile(.{ .file = b.path("./gd32e23x/std/src/gd32e23x_spi.c") });
    }
    if (b.option(bool, "adc", "Use peripheral adc. (default 'false')") orelse false) {
        elf.defineCMacro("GD32E23X_USE_ADC", null);
        elf.addCSourceFile(.{ .file = b.path("./gd32e23x/std/src/gd32e23x_adc.c") });
    }
    if (b.option(bool, "pmu", "Use peripheral pmu. (default 'false')") orelse false) {
        elf.defineCMacro("GD32E23X_USE_PMU", null);
        elf.addCSourceFile(.{ .file = b.path("./gd32e23x/std/src/gd32e23x_pmu.c") });
    }
    if (b.option(bool, "rtc", "Use peripheral rtc. (default 'false')") orelse false) {
        elf.defineCMacro("GD32E23X_USE_RTC", null);
        elf.addCSourceFile(.{ .file = b.path("./gd32e23x/std/src/gd32e23x_rtc.c") });
    }
    if (b.option(bool, "exti", "Use peripheral exti. (default 'false')") orelse false) {
        elf.defineCMacro("GD32E23X_USE_EXTI", null);
        elf.addCSourceFile(.{ .file = b.path("./gd32e23x/std/src/gd32e23x_exti.c") });
    }
    if (b.option(bool, "timer", "Use peripheral timer. (default 'false')") orelse false) {
        elf.defineCMacro("GD32E23X_USE_TIMER", null);
        elf.addCSourceFile(.{ .file = b.path("./gd32e23x/std/src/gd32e23x_timer.c") });
    }
    
    const option_include_dirs = b.option([]const u8, "inc", "The relative path to include dir for the program. (example './src;./src/lib' or './src;./src/lib;')");
    if (option_include_dirs) |include_dirs| {
        var it = std.mem.tokenizeAny(u8, include_dirs, ";\n ");
        while (it.next()) |include_dir| {
            if (enable_show_info) std.debug.print("User include dir: {s}\n", .{ include_dir });
            elf.addIncludePath(b.path(include_dir));
        }
    }

    const option_user_c_files = b.option([]const u8, "src", "The relative path to user c file for the program. (example './src/main.c;./src/lib.c' or './src/main.c;./src/lib.c;')");
    if (option_user_c_files) |user_c_files| {
        var it = std.mem.tokenizeAny(u8, user_c_files, ";\n ");
        while (it.next()) |user_c_file| {
            if (enable_show_info) std.debug.print("User C file: {s}\n", .{ user_c_file });
            elf.addCSourceFile(.{ .file = b.path(user_c_file) });
        }
    }

    const option_macros = b.option([]const u8, "macro", "The macro for the program. (example 'DEBUG;OPEN' ro 'DEBUG;OPEN;')");
    if (option_macros) |macros| {
        var it = std.mem.tokenizeAny(u8, macros, ";\n ");
        while (it.next()) |macro| {
            if (enable_show_info) std.debug.print("User macro: {s}\n", .{ macro });
            elf.defineCMacro(macro, null);
        }
    }

    const bin = b.addObjCopy(elf.getEmittedBin(), .{ .format = .bin });
    bin.step.dependOn(&elf.step);
    const copy_bin = b.addInstallBinFile(bin.getOutput(), b.fmt("{s}.bin", .{ std.mem.trimRight(u8, elf.name, std.fs.path.extension(elf.name)) }));
    b.default_step.dependOn(&copy_bin.step);
    b.installArtifact(elf);

    const clangd_emit = b.option(bool, "clangd", "Enable to generate clangd config file") orelse false;
    if (clangd_emit) {
        try clangd.CompileCommandsJson.generate(b, elf.root_module, .{});
    }
}

const clangd = struct {
    fn getZigRootPath(b: *std.Build) ![]const u8 {
        const zig_exe_path = try b.findProgram(&.{"zig"}, &.{});
        const zig_root_path = std.fs.path.dirname(zig_exe_path);
        return zig_root_path orelse error.Failed;
    }

    pub const CompileCommandsJson = struct {
        const Item = struct {
            arguments: []const []const u8,
            directory: []const u8,
            file: []const u8,
        };

        pub const GenerateOptions = struct {
            cstd: ?CStd = null,

            const CStd = union(enum) {
                // $zig_root_path$/lib/libc/include/$arch_os_abi$
                Libc: []const u8,
                // $zig_root_path$/lib/libcxx/include
                Libcxx,
            };
        };

        pub fn generate(
            b: *std.Build,
            module: std.Build.Module,
            options: GenerateOptions,
        ) !void {
            const systemIncludeDir: [3]?[]const u8 = blk: {
                var ret: [3]?[]const u8 = .{ null, null, null };
                if (getZigRootPath(b)) |zig_root_path| {
                    // FIXME Zig 与 Clangd 冲突
                    // const zig_cc_builtin_include_path = try std.fs.path.resolve(b.allocator, &[_][]const u8 {
                    //     zig_root_path,
                    //     "lib/include"
                    // });
                    // ret[0] = zig_cc_builtin_include_path;

                    if (options.cstd) |cstd| {
                        switch (cstd) {
                            .Libc => |arch_os_abi| {
                                const libc_include_path = try std.fs.path.resolve(b.allocator, &[_][]const u8 {
                                    zig_root_path,
                                    "lib/libc/include",
                                    arch_os_abi,
                                });
                                ret[1] = libc_include_path;
                            },
                            .Libcxx => {
                                ret[2] = try std.fs.path.resolve(b.allocator, &[_][]const u8 {
                                    zig_root_path,
                                    "lib/libcxx/include"
                                });
                            }
                        }
                    }
                } else |_| {
                    std.log.err("Failed to get zig_root_path\n", .{});
                }
                break :blk ret;
            };

            const cwd = try std.fs.cwd().realpathAlloc(b.allocator, ".");
            defer b.allocator.free(cwd);

            const c_macros = module.c_macros.items;
            const include_dirs = blk: {
                var ret = std.ArrayList([]const u8).init(b.allocator);
                for (module.include_dirs.items) |include_dir| {
                    switch (include_dir) {
                        .path,
                        .path_system,
                        .path_after,
                        .framework_path,
                        .framework_path_system => |p| {
                            try ret.append(p.getPath(b));
                        },
                        .other_step => {},
                        .config_header_step => {},
                    }
                }
                break :blk ret;
            };
            defer include_dirs.deinit();

            var data = std.ArrayList(Item).init(b.allocator);
            defer data.deinit();

            // 未对 Item 内存进行设计和管理（释放）
            for (module.link_objects.items) |link_object| {
                switch (link_object) {
                    else => {},
                    .c_source_file => |csf| {
                        const file_relative_path = try std.fs.path.relative(b.allocator, cwd, csf.file.getPath(b));

                        var arguments = std.ArrayList([]const u8).init(b.allocator);
                        try arguments.append("zig cc");                 // Compiler
                        try arguments.append(file_relative_path);       // SourceFile

                        for (csf.flags) |flag| {
                            try arguments.append(flag);
                        }

                        try arguments.append("-D__GNUC__");
                        for (c_macros) |c_macro| {
                            try arguments.append(c_macro);
                        }

                        for (systemIncludeDir) |sid| {
                            if (sid) |_sid| {
                                try arguments.append(b.fmt("-I{s}", .{_sid}));
                            }
                        }
                        for (include_dirs.items) |include_dir| {
                            const dir_relative = try std.fs.path.relative(b.allocator, cwd, include_dir);
                            try arguments.append(b.fmt("-I{s}", .{dir_relative}));
                        }

                        const item = Item {
                            .arguments = arguments.items,
                            .directory = cwd,
                            .file = file_relative_path,
                        };
                        try data.append(item);
                    }
                }
            }

            const json_string = try std.json.stringifyAlloc(b.allocator, data.items, .{
                .whitespace = .indent_4,
            });
            defer b.allocator.free(json_string);

            const json_file = try std.fs.cwd().createFile("compile_commands.json", .{});
            _ = try json_file.write(json_string);
        }
    };
};