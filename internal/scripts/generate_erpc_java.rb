#! ruby -I../core -I.
require 'rubygems'

unit_tests = [
#       'test_arbitrator',
#       'test_annotations', # Custom package
#       'test_arrays', # Custom package
        'test_binary',
        'test_builtin',
        'test_const',
#       'test_callbacks',
        'test_enums',
        'test_lists',
#       'test_struct' # Custom IDL file
        'test_typedef',
#       'test_unions'
    ]

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
    execute("#{erpc_path}/#{erpcgen_path}/erpcgen -g java -p io.github.embeddedrpc.erpc.tests.#{unit_test}.erpc_outputs -o #{erpc_path}/test/java_impl_tests/src/main/java/io/github/embeddedrpc/erpc/tests/#{unit_test}/ #{erpc_path}/test/#{unit_test}/#{unit_test}.erpc")
end

# Custom test for test_struct without uint64 type.
unit_test = 'test_struct'
execute("#{erpc_path}/#{erpcgen_path}/erpcgen -g java -p io.github.embeddedrpc.erpc.tests.#{unit_test}.erpc_outputs -o #{erpc_path}/test/java_impl_tests/src/main/java/io/github/embeddedrpc/erpc/tests/#{unit_test}/ #{erpc_path}/test/#{unit_test}/test_struct_java.erpc")

# Custom test for test_array with different package name.
unit_test = 'test_arrays'
execute("#{erpc_path}/#{erpcgen_path}/erpcgen -g java -p io.github.embeddedrpc.erpc.tests.#{unit_test} -o #{erpc_path}/test/java_impl_tests/src/main/java/io/github/embeddedrpc/erpc/tests/#{unit_test}/ #{erpc_path}/test/#{unit_test}/#{unit_test}.erpc")

unit_test = 'test_annotations'
execute("#{erpc_path}/#{erpcgen_path}/erpcgen -g java -p io.github.embeddedrpc.erpc.tests.#{unit_test} -o #{erpc_path}/test/java_impl_tests/src/main/java/io/github/embeddedrpc/erpc/tests/#{unit_test}/ #{erpc_path}/test/#{unit_test}/#{unit_test}.erpc")
