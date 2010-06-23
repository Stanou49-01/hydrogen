/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef SAMPLE_BUFFER_H
#define SAMPLE_BUFFER_H

#include <string>
#include <hydrogen/globals.h>
#include <hydrogen/Object.h>

namespace H2Core
{

class SampleVeloPan
{
public:
	struct SampleVeloVector
	{
		int m_SampleVeloframe;
		int m_SampleVelovalue;
	};

	std::vector<SampleVeloVector> m_Samplevolumen;

	struct SamplePanVector
	{
		int m_SamplePanframe;
		int m_SamplePanvalue;
	};

	std::vector<SamplePanVector> m_SamplePan;

	SampleVeloPan() {
		SampleVeloVector velovector;
		velovector.m_SampleVeloframe = -1;
		velovector.m_SampleVelovalue = -1;
		m_Samplevolumen.push_back( velovector );
		SamplePanVector panvector;
		panvector.m_SamplePanframe = -1;	
		panvector.m_SamplePanvalue = -1;
		m_SamplePan.push_back( panvector );
	}

	SampleVeloPan( const SampleVeloPan& velopan ) {
		m_Samplevolumen = velopan.m_Samplevolumen;
		m_SamplePan = velopan.m_SamplePan;
	}


};

class Sample : public Object
{
    H2_OBJECT
public:
	Sample(
		const QString& filename, 
        int frames,
		int sample_rate,
		float* data_L = 0,
		float* data_R = 0,
		bool sample_is_modified = false,
		const QString& sample_mode = "forward",
		int start_frame = 0,
		int end_frame = 0,
		int loop_frame = 0,
		int repeats = 0,
		SampleVeloPan velopan = SampleVeloPan(),
		bool use_rubber = false,
		float rubber_pitch = 0.0,
		float use_rubber_divider = 1.0,
		int use_rubber_c_settings = 4
        );

	~Sample();

	/**
     * load a sample from a file
     * \param filename the file to load audio data from
     */
	static Sample* load( const QString& filename );

	/// Loads an modified sample
	static Sample* load_edit_wave(
        const QString& filename,
        const int startframe,
        const int loppframe,
        const int endframe,
        const int loops,
        const QString loopmode,
        const bool use_rubberband,
        const float rubber_divider,
        const int rubber_c_settings,
        const float rubber_pitch );

	const QString get_filename() const          { return __filename; }
	void set_frames( int frames )               { __frames = frames; }
	int get_frames() const                      { return __frames; }
	int get_sample_rate() const                 { return __sample_rate; }
	int get_size() const                        { return __frames * sizeof( float ) * 2; }
	float* get_data_l() const                   { return __data_l; }
	float* get_data_r() const                   { return __data_r; }
	void set_is_modified( bool is_modified )    { __sample_is_modified = is_modified; }
	bool get_is_modified() const                { return __sample_is_modified; }
	//void set_sample_mode( QString sample_mode )         { __sample_mode = sample_mode; }
	QString get_sample_mode() const                     { return __sample_mode; }
	void set_start_frame( int start_frame )             { __start_frame = start_frame; }
	int get_start_frame() const                         { return __start_frame; }
	void set_end_frame( int end_frame )                 { __end_frame = end_frame; }
	int get_end_frame() const                           { return __end_frame; }
	void set_loop_frame( int loop_frame )               {  __loop_frame = loop_frame; }
	int get_loop_frame() const                          { return __loop_frame; }
	void set_repeats( int repeats )                     { __repeats = repeats; }
	int get_repeats() const                             { return __repeats; }
	void set_use_rubber( bool use_rubber )              { __use_rubber = use_rubber; }
	bool get_use_rubber() const                         { return __use_rubber; }
	void set_rubber_pitch( float rubber_pitch )         { __rubber_pitch = rubber_pitch; }
	float get_rubber_pitch() const                      { return __rubber_pitch; }
	void set_rubber_divider( float use_rubber_divider ) {__rubber_divider = use_rubber_divider; }
	float get_rubber_divider() const                    { return __rubber_divider; }
	void set_rubber_C_settings( int use_rubber_c_settings) { __rubber_C_settings = use_rubber_c_settings; }
	float get_rubber_C_settings() const                 { return __rubber_C_settings; }
	//void sampleEditProzess( Sample* Sample );
	//void setmod();

private:
    // TODO relative or absolute path ??
	QString __filename;		    ///< filename associated with this sample
	int __frames;		        ///< number of frames in this sample.
	int __sample_rate;		    ///< samplerate for this sample
	float *__data_l;	        ///< left channel data
	float *__data_r;	        ///< right channel data
	bool __sample_is_modified;	///< true if sample is modified
	QString __sample_mode;		///< loop mode
	int __start_frame;		    ///< start frame
	int __end_frame; 		    ///< sample end frame
	int __loop_frame;		    ///< beginn of the loop section
	int __repeats;			    ///< repeats from the loop section
public:
	SampleVeloPan __velo_pan;	///< volume and pan vector
private:
	bool __use_rubber;		    ///< use the rubberband bin
	float __rubber_pitch;		///< rubberband pitch
	float __rubber_divider;		///< the divider to calculate the ratio
	int __rubber_C_settings;	///< the rubberband "crispness" levels
	//static int __total_used_bytes;

	/**
     * load sample data using libsndfile
     * \param filename the file to load audio data from
     */
	static Sample* libsndfile_load( const QString& filename );

};

};

#endif