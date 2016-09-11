/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licences for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licencing:
*
*   http://www.hartinstruments.net/hise/
*
*   HISE is based on the JUCE library,
*   which also must be licenced for commercial applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

HiseEvent::HiseEvent(const MidiMessage& message)
{
	const uint8* data = message.getRawData();

	channel = message.getChannel();

	if (message.isNoteOn()) type = Type::NoteOn;
	else if (message.isNoteOff()) type = Type::NoteOff;
	else if (message.isPitchWheel()) type = Type::PitchBend;
	else if (message.isController()) type = Type::Controller;
	else if (message.isChannelPressure() || message.isAftertouch()) type = Type::Aftertouch;
	else if (message.isAllNotesOff() || message.isAllSoundOff()) type = Type::AllNotesOff;
	else
	{
		// unsupported Message type, add another...
		jassertfalse;
	}
	
	number = data[1];
	value = data[2];
}

double HiseEvent::getPitchFactorForEvent() const
{
	if (semitones == 0 && cents == 0) return 1.0;

	const float detuneFactor = (float)semitones + (float)cents / 100.0f;

	return (double)Modulation::PitchConverters::octaveRangeToPitchFactor(detuneFactor);
}

HiseEventBuffer::HiseEventBuffer()
{
	clear();
}

void HiseEventBuffer::clear()
{
    memset(buffer, 0, HISE_EVENT_BUFFER_SIZE * sizeof(HiseEvent));
    
	numUsed = 0;
}

void HiseEventBuffer::addEvent(const HiseEvent& hiseEvent)
{
	if (numUsed >= HISE_EVENT_BUFFER_SIZE)
	{
		// Buffer full..
		jassertfalse;
		return;
	}

	if (numUsed == 0)
	{
		insertEventAtPosition(hiseEvent, 0);
		return;
	}

	int currentSamplePosition = 0;

	bool rightPosition = false;

	jassert(numUsed < HISE_EVENT_BUFFER_SIZE);

    const int numToLookFor = jmin<int>(numUsed, HISE_EVENT_BUFFER_SIZE);
    
	for (int i = 0; i < numToLookFor; i++)
	{
		const int timestampInBuffer = buffer[i].getTimeStamp();
		const int messageTimestamp = hiseEvent.getTimeStamp();

		if (messageTimestamp <= timestampInBuffer)
		{
			insertEventAtPosition(hiseEvent, timestampInBuffer == messageTimestamp ? i + 1 : i);
			return;
		}
	}

	insertEventAtPosition(hiseEvent, numUsed);
}

void HiseEventBuffer::addEvent(const MidiMessage& midiMessage, int sampleNumber)
{
	HiseEvent e(midiMessage);
	e.setTimeStamp(sampleNumber);

	addEvent(e);
}

void HiseEventBuffer::addEvents(const MidiBuffer& otherBuffer)
{
	clear();

	MidiMessage m;
	int samplePos;

	int index = 0;

	MidiBuffer::Iterator it(otherBuffer);

	while (it.getNextEvent(m, samplePos))
	{
		jassert(index < HISE_EVENT_BUFFER_SIZE);

		buffer[index] = HiseEvent(m);
		buffer[index].setTimeStamp(samplePos);

		numUsed++;

		if (numUsed >= HISE_EVENT_BUFFER_SIZE)
		{
			// Buffer full..
			jassertfalse;
			return;
		}

		index++;
	}
}


void HiseEventBuffer::subtractFromTimeStamps(int delta)
{
	if (numUsed == 0) return;

	for (int i = 0; i < numUsed; i++)
	{
		buffer[i].addToTimeStamp(-delta);
	}
}

void HiseEventBuffer::moveEventsBelow(HiseEventBuffer& targetBuffer, int highestTimestamp)
{
	if (numUsed == 0) return;

	HiseEventBuffer::Iterator iter(*this);

	int numCopied = 0;

	while (HiseEvent* e = iter.getNextEventPointer())
	{
		if (e->getTimeStamp() < highestTimestamp)
		{
			targetBuffer.addEvent(*e);
			numCopied++;
		}
		else
		{
			break;
		}
	}

	const int numRemaining = numUsed - numCopied;

	for (int i = 0; i < numRemaining; i++)
		buffer[i] = buffer[i + numCopied];

	HiseEvent::clear(buffer + numRemaining, numCopied);

	numUsed = numRemaining;
}

void HiseEventBuffer::moveEventsAbove(HiseEventBuffer& targetBuffer, int lowestTimestamp)
{
	if (numUsed == 0 || (buffer[numUsed - 1].getTimeStamp() < lowestTimestamp)) 
		return; // Skip the work if no events with bigger timestamps

	int indexOfFirstElementToMove = -1;

	for (int i = 0; i < numUsed; i++)
	{
		if (buffer[i].getTimeStamp() >= lowestTimestamp)
		{
			indexOfFirstElementToMove = i;
			break;
		}
	}

	if (indexOfFirstElementToMove == -1) return;

	for (int i = indexOfFirstElementToMove; i < numUsed; i++)
	{
		targetBuffer.addEvent(buffer[i]);
	}

	HiseEvent::clear(buffer + indexOfFirstElementToMove, numUsed - indexOfFirstElementToMove);

	numUsed = indexOfFirstElementToMove;
}

void HiseEventBuffer::copyFrom(const HiseEventBuffer& otherBuffer)
{
    const int eventsToCopy = jmin<int>(otherBuffer.numUsed, HISE_EVENT_BUFFER_SIZE);
    
	memcpy(buffer, otherBuffer.buffer, sizeof(HiseEvent) * eventsToCopy);
    memset(buffer + eventsToCopy, 0, (HISE_EVENT_BUFFER_SIZE - eventsToCopy) * sizeof(HiseEvent));
    
	jassert(otherBuffer.numUsed < HISE_EVENT_BUFFER_SIZE);

	numUsed = otherBuffer.numUsed;
}

HiseEventBuffer::Iterator::Iterator(HiseEventBuffer &b) :
buffer(&b),
constBuffer(nullptr),
index(0)
{

}


HiseEventBuffer::Iterator::Iterator(const HiseEventBuffer& b) :
constBuffer(&b),
buffer(nullptr),
index(0)
{

}

bool HiseEventBuffer::Iterator::getNextEvent(HiseEvent& b, int &samplePosition)
{
	while (index < buffer->numUsed && buffer->buffer[index].isIgnored())
	{
		index++;
		jassert(index < HISE_EVENT_BUFFER_SIZE);
	}
		
	if (index < buffer->numUsed)
	{
		b = buffer->buffer[index];
		samplePosition = b.getTimeStamp();
		index++;
		return true;
	}
	else
		return false;
}


bool HiseEventBuffer::Iterator::getNextEvent(HiseEvent& b, int &samplePosition) const
{
	while (index < constBuffer->numUsed && constBuffer->buffer[index].isIgnored())
	{
		index++;
		jassert(index < HISE_EVENT_BUFFER_SIZE);
	}

	if (index < constBuffer->numUsed)
	{
		b = constBuffer->buffer[index];
		samplePosition = b.getTimeStamp();
		index++;
		return true;
	}
	else
		return false;
}

HiseEvent* HiseEventBuffer::Iterator::getNextEventPointer(bool skipArtificialNotes/*=false*/)
{
	while (index < buffer->numUsed && (skipArtificialNotes && buffer->buffer[index].isArtificial()) || buffer->buffer[index].isIgnored())
	{
		index++;
		jassert(index < HISE_EVENT_BUFFER_SIZE);
	}

	if (index < buffer->numUsed)
	{
		return &buffer->buffer[index++];

	}
	else
	{
		return nullptr;
	}
}


const HiseEvent* HiseEventBuffer::Iterator::getNextEventPointer(bool skipArtificialNotes /*= false*/) const
{
	while (index < constBuffer->numUsed && (skipArtificialNotes && constBuffer->buffer[index].isArtificial()) || constBuffer->buffer[index].isIgnored())
	{
		index++;
		jassert(index < HISE_EVENT_BUFFER_SIZE);
	}

	if (index < constBuffer->numUsed)
	{
		return &constBuffer->buffer[index++];

	}
	else
	{
		return nullptr;
	}
}

void HiseEventBuffer::insertEventAtPosition(const HiseEvent& e, int positionInBuffer)
{
	if (numUsed == 0)
	{
		buffer[0] = HiseEvent(e);

		numUsed = 1;

		return;
	}

	if (numUsed > positionInBuffer)
	{
		for (int i = jmin<int>(numUsed-1, HISE_EVENT_BUFFER_SIZE-2); i >= positionInBuffer; i--)
		{
			jassert(i + 1 < HISE_EVENT_BUFFER_SIZE);

			buffer[i + 1] = buffer[i];
		}
	}

    if(positionInBuffer < HISE_EVENT_BUFFER_SIZE)
    {
        buffer[positionInBuffer] = HiseEvent(e);
        numUsed++;
    }
    else
    {
        jassertfalse;
    }
    
    
	

	
}

void HiseEventBuffer::EventIdHandler::handleEventIds()
{
	for (int i = 0; i < masterBuffer.numUsed; i++)
	{
		HiseEvent* m = &masterBuffer.buffer[i];
		if (m->isNoteOn())
		{
			m->setEventId(currentEventId);
			noteOnEvents[m->getNoteNumber()] = HiseEvent(*m);

			currentEventId++;
		}
		else if (m->isNoteOff())
		{
			uint32 id = noteOnEvents[m->getNoteNumber()].getEventId();
			m->setEventId(id);
		}
	}
}

const HiseEvent* HiseEventBuffer::EventIdHandler::getNoteOnEventFor(const HiseEvent &noteOffEvent) const
{
	jassert(noteOffEvent.isNoteOff());

	const int noteNumber = noteOffEvent.getNoteNumber();

	const HiseEvent *m = &noteOnEvents[noteNumber];

	jassert(noteOffEvent.getEventId() == m->getEventId());

	return m;
}

int HiseEventBuffer::EventIdHandler::requestEventIdForArtificialNote() noexcept
{
	return currentEventId++;
}


