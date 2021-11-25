if defined?(Ractor)
  describe 'xxhash inside ractor' do
    it 'calculates inside ractor' do
      assert_equal(Ractor.new { Digest::XXH32.hexdigest('hello') }.take, 'fb0077f9')
    end
  end
end