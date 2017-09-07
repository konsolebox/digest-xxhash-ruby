require 'csv'
require 'minitest/autorun'

# To show more verbose messages, install minitest-reporters and uncomment the
# following lines:
#
# require "minitest/reporters"
# Minitest::Reporters.use! [Minitest::Reporters::SpecReporter.new]

TEST_DIR = File.dirname(__FILE__)
require File.join(TEST_DIR, %w{ .. lib digest xxhash })

def get_repeated_0x00_to_0xff(length)
  hex = (0..0xff).to_a.map{ |e| sprintf "%2x", e }.join
  str = [hex].pack('H*')
  cycles = (Float(length) / str.size).ceil
  [str].cycle(cycles).to_a.join[0...length]
end

[Digest::XXH32, Digest::XXH64].each do |klass|
  describe klass do
    it "produces correct types of digest outputs" do
      klass.digest("").must_be_instance_of String
      klass.hexdigest("").must_be_instance_of String
      klass.idigest("").must_be_instance_of Integer
      klass.new.digest("").must_be_instance_of String
      klass.new.hexdigest("").must_be_instance_of String
      klass.new.idigest("").must_be_instance_of Integer
    end

    it "produces similar output with its digest, hexdigest and idigest methods" do
      digest = klass.digest("abcd")
      klass.new.digest("abcd").must_equal digest
      klass.new.update("ab").update("cd").digest.must_equal digest
      klass.new.update("ab").update("cd").digest!.must_equal digest
      klass.new.reset.update("ab").update("cd").digest!.must_equal digest

      hexdigest = klass.hexdigest("abcd")
      klass.new.hexdigest("abcd").must_equal hexdigest
      klass.new.update("ab").update("cd").hexdigest.must_equal hexdigest
      klass.new.update("ab").update("cd").hexdigest!.must_equal hexdigest
      klass.new.reset.update("ab").update("cd").hexdigest!.must_equal hexdigest

      idigest = klass.idigest("abcd")
      klass.new.idigest("abcd").must_equal idigest
      klass.new.update("ab").update("cd").idigest.must_equal idigest
      klass.new.update("ab").update("cd").idigest!.must_equal idigest
      klass.new.reset.update("ab").update("cd").idigest!.must_equal idigest

      digest_enc = digest.unpack('H*').pop
      hexdigest.must_equal digest_enc

      idigest_enc = "%08x" % idigest
      hexdigest.must_equal idigest_enc
    end
  end
end

CSV.foreach(File.join(TEST_DIR, 'test.vectors'), col_sep: '|').with_index(1) do |csv, line_num|
  bit_size, msg_method, msg_length, seed, sum = csv

  case msg_method
  when 'null'
    msg = ''
  when '0x00_to_0xff'
    msg = get_repeated_0x00_to_0xff(msg_length.to_i)
  else
    raise "Invalid specified message generation method in 'test.vectors', line #{line_num}."
  end

  case bit_size.to_i
  when 32
    klass = Digest::XXH32
  when 64
    klass = Digest::XXH64
  else
    raise "Invalid specified bit size in 'test.vectors', line #{line_num}."
  end

  describe klass do
    describe "using #{msg_method}(#{msg_length}) as message generator, and #{seed} as seed" do
      it "should produce #{sum}"  do
        klass.hexdigest(msg, seed).must_equal sum
      end
    end
  end
end
