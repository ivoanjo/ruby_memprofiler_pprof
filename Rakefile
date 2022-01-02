require "bundler/gem_tasks"
require "rake/extensiontask"
require "rake/testtask"
require "bump/tasks"
require "private_gem/tasks"

gemspec = Gem::Specification.load("ruby_memprofiler_pprof.gemspec.gemspec")
task :compile => :protoc
task :clean => :clean_protoc
Rake::ExtensionTask.new do |ext|
  ext.name = "ruby_memprofiler_pprof_ext"
  ext.source_pattern = "*.{c,h}"
  ext.ext_dir = "ext/ruby_memprofiler_pprof"
  ext.lib_dir = "lib/ruby_memprofiler_pprof"
  ext.gem_spec = gemspec
end
Rake::TestTask.new(:test) {}

task default: [:compile]

task :protoc => Rake::FileList[
  'lib/ruby_memprofiler_pprof/pb/pprof_pb.rb',
  'ext/ruby_memprofiler_pprof/pprof.pb.h',
  'ext/ruby_memprofiler_pprof/pprof.pb.cc',
]

file 'lib/ruby_memprofiler_pprof/pb/pprof_pb.rb' => Rake::FileList["proto/pprof.proto"] do
  sh 'protoc',  '--proto_path=proto', '--ruby_out=lib/ruby_memprofiler_pprof/pb', 'proto/pprof.proto'
end

file 'ext/ruby_memprofiler_pprof/pprof.pb.cc' => 'ext/ruby_memprofiler_pprof/pprof.pb.h'
file 'ext/ruby_memprofiler_pprof/pprof.pb.h' => Rake::FileList["proto/pprof.proto"] do
  sh 'protoc',  '--proto_path=proto', '--cpp_out=ext/ruby_memprofiler_pprof/', 'proto/pprof.proto'
end

task :clean_protoc do
    pb_files = Dir['ext/ruby_memprofiler_pprof/*.pb.{cc,h}']
    rm_f pb_files if pb_files.any?
end

# These Rake tasks run the protobuf compiler to generate the .upb.c code. These are not run as part of
# the gem install; the generated protobufs are actually checked in.
PROTOC_GEN_UPB = 'ext/ruby_memprofiler_pprof/upb/bazel-bin/upbc/protoc-gen-upb'
PROTO_OUT = Rake::FileList[
  'ext/ruby_memprofiler_pprof/pprof.upb.c',
  'ext/ruby_memprofiler_pprof/pprof.upb.h',
]
file PROTOC_GEN_UPB do
  cd 'ext/ruby_memprofiler_pprof/upb/upbc' do
    sh 'bazel', 'build', 'protoc-gen-upb'
  end
end

file PROTO_OUT => PROTOC_GEN_UPB do
  sh 'protoc', '--proto_path=proto', "--plugin=#{PROTOC_GEN_UPB}", "--upb_out=ext/ruby_memprofiler_pprof"
end
