#pragma once
#include "framework.h"

#include "IWSStream.hpp"

struct wdc3_db2_header
{
	uint32_t magic;                  // 'WDC3'
	uint32_t record_count;           // this is for all sections combined now
	uint32_t field_count;
	uint32_t record_size;
	uint32_t string_table_size;      // this is for all sections combined now
	uint32_t table_hash;             // hash of the table name
	uint32_t layout_hash;            // this is a hash field that changes only when the structure of the data changes
	uint32_t min_id;
	uint32_t max_id;
	uint32_t locale;                 // as seen in TextWowEnum
	uint16_t flags;                  // possible values are listed in Known Flag Meanings
	uint16_t id_index;               // this is the index of the field containing ID values; this is ignored if flags & 0x04 != 0
	uint32_t total_field_count;      // from WDC1 onwards, this value seems to always be the same as the 'field_count' value
	uint32_t bitpacked_data_offset;  // relative position in record where bitpacked data begins; not important for parsing the file
	uint32_t lookup_column_count;
	uint32_t field_storage_info_size;
	uint32_t common_data_size;
	uint32_t pallet_data_size;
	uint32_t section_count;          // new to WDC2, this is number of sections of data
};

// a section = records + string block + id list + copy table + offset map + offset map id list + relationship map
struct wdc3_section_header
{
	uint64_t tact_key_hash;          // TactKeyLookup hash
	uint32_t file_offset;            // absolute position to the beginning of the section
	uint32_t record_count;           // 'record_count' for the section
	uint32_t string_table_size;      // 'string_table_size' for the section
	uint32_t offset_records_end;     // Offset to the spot where the records end in a file with an offset map structure;
	uint32_t id_list_size;           // Size of the list of ids present in the section
	uint32_t relationship_data_size; // Size of the relationship data in the section
	uint32_t offset_map_id_count;    // Count of ids present in the offset map in the section
	uint32_t copy_table_count;       // Count of the number of deduplication entries (you can multiply by 8 to mimic the old 'copy_table_size' field)
};

struct field_structure
{
	int16_t size;                   // size in bits as calculated by: byteSize = (32 - size) / 8; this value can be negative to indicate field sizes larger than 32-bits
	uint16_t position;              // position of the field within the record, relative to the start of the record
};


enum class field_compression
	: uint32_t
{
	// None -- usually the field is a 8-, 16-, 32-, or 64-bit integer in the record data. But can contain 96-bit value representing 3 floats as well
	field_compression_none,
	// Bitpacked -- the field is a bitpacked integer in the record data.  It
	// is field_size_bits long and starts at field_offset_bits.
	// A bitpacked value occupies
	//   (field_size_bits + (field_offset_bits & 7) + 7) / 8
	// bytes starting at byte
	//   field_offset_bits / 8
	// in the record data.  These bytes should be read as a little-endian value,
	// then the value is shifted to the right by (field_offset_bits & 7) and
	// masked with ((1ull << field_size_bits) - 1).
	field_compression_bitpacked,
	// Common data -- the field is assumed to be a default value, and exceptions
	// from that default value are stored in the corresponding section in
	// common_data as pairs of { uint32_t record_id; uint32_t value; }.
	field_compression_common_data,
	// Bitpacked indexed -- the field has a bitpacked index in the record data.
	// This index is used as an index into the corresponding section in
	// pallet_data.  The pallet_data section is an array of uint32_t, so the index
	// should be multiplied by 4 to obtain a byte offset.
	field_compression_bitpacked_indexed,
	// Bitpacked indexed array -- the field has a bitpacked index in the record
	// data.  This index is used as an index into the corresponding section in
	// pallet_data.  The pallet_data section is an array of uint32_t[array_count],
	//
	field_compression_bitpacked_indexed_array,
	// Same as field_compression_bitpacked
	field_compression_bitpacked_signed,
};

struct field_storage_info
{
	uint16_t          field_offset_bits;
	uint16_t          field_size_bits; // very important for reading bitpacked fields; size is the sum of all array pieces in bits - for example, uint32[3] will appear here as '96'
	// additional_data_size is the size in bytes of the corresponding section in
	// common_data or pallet_data.  These sections are in the same order as the
	// field_info, so to find the offset, add up the additional_data_size of any
	// previous fields which are stored in the same block (common_data or
	// pallet_data).
	uint32_t          additional_data_size;
	field_compression storage_type;
	union
	{
		struct
		{
			uint32_t bitpacking_offset_bits; // not useful for most purposes; formula they use to calculate is bitpacking_offset_bits = field_offset_bits - (header.bitpacked_data_offset * 8)
			uint32_t bitpacking_size_bits; // not useful for most purposes
			uint32_t flags; // known values - 0x01: sign-extend (signed)
		} bitpacked;
		struct
		{
			uint32_t default_value;
			uint32_t unk_or_unused2;
			uint32_t unk_or_unused3;
		} common;
		struct
		{
			uint32_t bitpacking_offset_bits; // not useful for most purposes; formula they use to calculate is bitpacking_offset_bits = field_offset_bits - (header.bitpacked_data_offset * 8)
			uint32_t bitpacking_size_bits; // not useful for most purposes
			uint32_t unk_or_unused3;
		} indexed;
		struct
		{
			uint32_t bitpacking_offset_bits; // not useful for most purposes; formula they use to calculate is bitpacking_offset_bits = field_offset_bits - (header.bitpacked_data_offset * 8)
			uint32_t bitpacking_size_bits; // not useful for most purposes
			uint32_t array_count;
		} indexed_array;
	};
};

enum class db2_flags
{
	has_offset_map = 0x1,
	has_relationship_data = 0x2,
	has_non_inline_ids = 0x4,
	is_bitpacked = 0x10,
};

class db2;
class db2_field;

class db2_record
{
	uint32_t m_index = 0;
	db2* m_db2 = nullptr;
public:
	db2_record(db2* db2, uint32_t index);

	db2_field GetField(uint32_t i);
	db2_field operator[](uint32_t i);
	uint32_t GetCount();
};

struct db2_section
{
	std::span<uint8_t> m_record_data;
	std::span<char> m_string_data;
	std::span<uint32_t> m_id_list;
	uint32_t m_offset = 0;
	wdc3_section_header* m_hdr = nullptr;
	uint32_t m_recordSize = 0;

	db2_section(wdc3_db2_header* hdr, std::span<uint8_t> buffer, wdc3_section_header* section_hdr);

	// index is relative to this section
	uint8_t* GetRecordData(uint32_t i);
	// index is relative to this section
	char* GetStringData(uint32_t ofs);
};

struct db2_field_info_ex
{
	field_storage_info m_storage_info;
	std::unordered_map<uint32_t, uint32_t> m_common_data;
	std::span<uint32_t> m_pallet_data;

	db2_field_info_ex(field_storage_info& storage_info, std::span<uint32_t> common_data, std::span<uint32_t> pallet_data)
	{
		m_storage_info = storage_info;
		m_common_data.reserve(common_data.size() / 2);
		for (int i = 0; i < common_data.size(); i += 2)
			m_common_data.emplace(common_data[i], common_data[i + 1]);

		m_pallet_data = pallet_data;
	}
};

class db2_record;
class db2
{
	db2() = delete;
	db2(const db2&) = delete;
	db2& operator=(const db2&) = delete;

	std::vector<uint8_t> m_buffer;
	wdc3_db2_header* m_hdr = nullptr;
	std::span<field_structure> m_fields;
	std::vector<db2_field_info_ex> m_field_infos;
	std::span<char> m_pallet_data;
	std::span<char> m_common_data;
	std::vector<db2_section> m_sections;
	std::unordered_map<uint32_t, uint32_t> m_id_map;

	friend db2_field;
	friend db2_record;

	db2_section* GetSectionFromRecordIndex(uint32_t i);
	uint8_t* GetRecordData(uint32_t recordIndex, uint32_t* sectionIndex = nullptr);
public:
	db2(IWSStream& file);

	db2_record GetRecordById(uint32_t id);
	db2_record GetRecord(uint32_t i);
	db2_record operator[](uint32_t i);
	uint32_t GetCount();
};

class db2_field
{
	db2* m_db2 = nullptr;
	uint32_t m_recordIndex = 0;
	uint32_t m_fieldIndex = 0;

	template<typename T>
	T GetBitpackedValue(db2_field_info_ex& info)
	{
		auto data = m_db2->GetRecordData(m_recordIndex);
		T value = 0;
		int count = 0;
		int remaining = info.m_storage_info.field_size_bits;
		int i = info.m_storage_info.field_offset_bits / 8;
		int ofs = info.m_storage_info.field_offset_bits % 8;
		while (remaining > 0)
		{
			int a = std::min<int>(remaining, 8 - ofs);
			uint8_t mask = 0xFF >> (8 - a);
			value |= ((T)((data[i] >> ofs) & mask)) << count;
			++i;
			count += a;
			remaining -= a;
			ofs = 0;
		}
		// sign extend
		if (info.m_storage_info.storage_type == field_compression::field_compression_bitpacked && info.m_storage_info.bitpacked.flags & 0x1)
			value |= ~T(0) << count;

		return value;
	}
	template<typename T>
	T AsT()
	{
		bool has_non_inline_ids = (m_db2->m_hdr->flags & static_cast<uint32_t>(db2_flags::has_non_inline_ids));
		if (m_fieldIndex == 0 && has_non_inline_ids)
			return static_cast<T>(m_db2->GetSectionFromRecordIndex(m_recordIndex)->m_id_list[m_recordIndex]);

		uint32_t fieldIndex = has_non_inline_ids ? m_fieldIndex - 1 : m_fieldIndex;

		auto& info = m_db2->m_field_infos[fieldIndex];

		switch (info.m_storage_info.storage_type)
		{
		case field_compression::field_compression_bitpacked_signed:
		case field_compression::field_compression_bitpacked:
		case field_compression::field_compression_bitpacked_indexed:
		case field_compression::field_compression_none:
		{
			auto value = GetBitpackedValue<T>(info);

			if (info.m_storage_info.storage_type == field_compression::field_compression_bitpacked_indexed)
				return m_db2->m_field_infos[fieldIndex].m_pallet_data[value];

			return value;
		}
		case field_compression::field_compression_bitpacked_indexed_array:
			throw std::exception();
		case field_compression::field_compression_common_data:
		{
			auto& common_data_map = m_db2->m_field_infos[fieldIndex].m_common_data;
			auto it = common_data_map.find(m_recordIndex);
			if (it != common_data_map.end())
				return static_cast<T>(it->second);

			return static_cast<T>(info.m_storage_info.common.default_value);
		}
		break;
		}
	}

	template<typename T, size_t size>
	std::array<T, size> AsTArray()
	{
		bool has_non_inline_ids = (m_db2->m_hdr->flags & static_cast<uint32_t>(db2_flags::has_non_inline_ids));
		if (m_fieldIndex == 0 && has_non_inline_ids)
			throw std::exception();

		uint32_t fieldIndex = has_non_inline_ids ? m_fieldIndex - 1 : m_fieldIndex;

		auto& info = m_db2->m_field_infos[fieldIndex];

		switch (info.m_storage_info.storage_type)
		{
		case field_compression::field_compression_bitpacked_indexed_array:
		{
			auto value = GetBitpackedValue<T>(info);
			std::array<T, size> a;
			if (size != m_db2->m_field_infos[fieldIndex].m_storage_info.indexed_array.array_count)
				throw std::exception();
			for(int i = 0; i < size; ++i)
				a[i] = m_db2->m_field_infos[fieldIndex].m_pallet_data[value * size + i];
			return a;
		}
		default:
			throw std::exception();
		}
	}
public:
	db2_field(db2* db2, uint32_t recordIndex, uint32_t fieldIndex)
		: m_db2(db2),
		m_recordIndex(recordIndex),
		m_fieldIndex(fieldIndex)
	{

	}

	int8_t AsInt8()
	{
		return AsT<int8_t>();
	}
	template<int size>
	std::array<int8_t, size> AsInt8Array()
	{
		return AsTArray<int8_t, size>();
	}
	uint8_t AsUInt8()
	{
		return AsT<uint8_t>();
	}
	template<int size>
	std::array<uint8_t, size> AsUInt8Array()
	{
		return AsTArray<uint8_t, size>();
	}
	int32_t AsInt32()
	{
		return AsT<int32_t>();
	}
	template<int size>
	std::array<int32_t, size> AsInt32Array()
	{
		return AsTArray<int32_t, size>();
	}
	uint32_t AsUInt32()
	{
		return AsT<uint32_t>();
	}
	template<int size>
	std::array<uint32_t, size> AsUInt32Array()
	{
		return AsTArray<uint32_t, size>();
	}
	int64_t AsInt64()
	{
		return AsT<int64_t>();
	}
	template<int size>
	std::array<int64_t, size> AsInt64Array()
	{
		return AsTArray<int64_t, size>();
	}
	uint64_t AsUInt64()
	{
		return AsT<uint64_t>();
	}
	template<int size>
	std::array<uint64_t, size> AsUInt64Array()
	{
		return AsTArray<uint64_t, size>();
	}
	float AsFloat()
	{
		uint32_t v = AsT<uint32_t>();
		return *reinterpret_cast<float*>(&v);
	}
	template<int size>
	std::array<float, size> AsFloatArray()
	{
		return AsTArray<float, size>();
	}
	std::string_view AsString()
	{
		uint32_t v = AsUInt32();
		auto sec = m_db2->GetSectionFromRecordIndex(m_recordIndex);
		auto ofs = sec->GetRecordData(m_recordIndex) + v - reinterpret_cast<uint8_t*>(sec->GetStringData(0));
		return sec->GetStringData(ofs);
	}
};