/*  HISE Lossless Audio Codec
*	�2017 Christoph Hart
*
*	Redistribution and use in source and binary forms, with or without modification,
*	are permitted provided that the following conditions are met:
*
*	1. Redistributions of source code must retain the above copyright notice,
*	   this list of conditions and the following disclaimer.
*
*	2. Redistributions in binary form must reproduce the above copyright notice,
*	   this list of conditions and the following disclaimer in the documentation
*	   and/or other materials provided with the distribution.
*
*	3. All advertising materials mentioning features or use of this software must
*	   display the following acknowledgement:
*	   This product includes software developed by Hart Instruments
*
*	4. Neither the name of the copyright holder nor the names of its contributors may be used
*	   to endorse or promote products derived from this software without specific prior written permission.
*
*	THIS SOFTWARE IS PROVIDED BY CHRISTOPH HART "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
*	BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*	DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
*	GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

HiseLosslessAudioFormatReader::HiseLosslessAudioFormatReader(InputStream* input_) :
	AudioFormatReader(input_, "HLAC"),
	header(input)
{
	numChannels = header.getNumChannels();
	sampleRate = header.getSampleRate();
	bitsPerSample = header.getBitsPerSample();
	lengthInSamples = header.getBlockAmount() * COMPRESSION_BLOCK_SIZE;
	usesFloatingPointData = true;

	decoder.setupForDecompression();
}

bool HiseLosslessAudioFormatReader::readSamples(int** destSamples, int numDestChannels, int startOffsetInDestBuffer, int64 startSampleInFile, int numSamples)
{
	ignoreUnused(startSampleInFile);
	ignoreUnused(numDestChannels);
	ignoreUnused(startOffsetInDestBuffer);

	bool isStereo = destSamples[1] != nullptr;

	if (isStereo)
	{
		float** destinationFloat = reinterpret_cast<float**>(destSamples);

		AudioSampleBuffer b(destinationFloat, 2, numSamples);

		decoder.decode(b, *input);

	}
	else
	{
		float* destinationFloat = reinterpret_cast<float*>(destSamples[0]);

		AudioSampleBuffer b(&destinationFloat, 1, numSamples);

		decoder.decode(b, *input);
	}

	return true;
}


void HiseLosslessAudioFormatReader::setTargetAudioDataType(AudioDataConverters::DataFormat dataType)
{
	usesFloatingPointData = (dataType == AudioDataConverters::DataFormat::float32BE) ||
		(dataType == AudioDataConverters::DataFormat::float32LE);
}
