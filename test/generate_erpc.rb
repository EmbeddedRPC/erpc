#! ruby -I../core -I.
require 'rubygems'

unit_tests = ['test_annotations','test_arrays','test_binary','test_builtin','test_const','test_enums','test_lists','test_struct','test_typedef','test_unions']

erpcgen_path = ARGV[0]
transport = ARGV[1]
board = ARGV[2]
core = ARGV[3]
erpc_path = ARGV[4]
#tool = ARGV[5]

def execute(command)
    puts "exec #{command}"
    $stdout.flush
    system("#{command}")
    $stdout.flush
    if ($?.exitstatus != 0)
        puts "error occurred, press enter key to quit" if (!ARGV.include?('nopause'))
        abort "1"
    end
end

unit_tests.each do | unit_test |
    execute("#{erpc_path}/#{erpcgen_path}/erpcgen -o #{erpc_path}/test/#{unit_test}/#{board}/#{transport} #{erpc_path}/test/#{unit_test}/#{unit_test}.erpc")
    execute("#{erpc_path}/#{erpcgen_path}/erpcgen -o #{erpc_path}/test/#{unit_test}/#{board}/#{transport} #{erpc_path}/test/common/unit_test_common.erpc")
end
