#! ruby -I../core -I.
require 'rubygems'

unit_tests = ['test_arbitrator','test_annotations','test_arrays','test_binary','test_builtin','test_const','test_callbacks','test_enums','test_lists','test_struct','test_typedef','test_unions']

erpcgen_path = ARGV[0]
erpc_path = ARGV[1]

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
    execute("#{erpc_path}/#{erpcgen_path}/erpcgen -gpy -o #{erpc_path}/test/python_impl_tests/scripts/service/#{unit_test}/ #{erpc_path}/test/#{unit_test}/#{unit_test}.erpc")
end
