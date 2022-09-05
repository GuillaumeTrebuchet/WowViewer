#include "framework.h"
#include "db2.hpp"


db2_section* db2::GetSectionFromRecordIndex(uint32_t i)
{
	uint32_t sum = 0;
	for (auto& it : m_sections)
	{
		if (i < sum + it.m_hdr->record_count)
			return &it;
		sum += it.m_hdr->record_count;
	}
	return nullptr;
}
uint8_t* db2::GetRecordData(uint32_t recordIndex, uint32_t* sectionIndex)
{
	uint32_t sum = 0;
	for (int i = 0; i < m_sections.size(); ++i)
	{
		if (recordIndex < sum + m_sections[i].m_hdr->record_count)
		{
			if (sectionIndex)
				*sectionIndex = i;

			return m_sections[i].GetRecordData(recordIndex - sum);
		}
		sum += m_sections[i].m_hdr->record_count;
	}
	return nullptr;
}

db2::db2(IWSStream& file)
{
	if (file.GetLength() > 10 * 1024 * 1024)
		throw std::exception("db2 file too big");

	m_buffer.resize(file.GetLength());
	file.Read(&m_buffer[0], m_buffer.size());

	uint32_t ofs = 0;
	m_hdr = reinterpret_cast<wdc3_db2_header*>(&m_buffer[ofs]);
	ofs += sizeof(*m_hdr);

	std::span<wdc3_section_header> sectionHeaders(reinterpret_cast<wdc3_section_header*>(&m_buffer[ofs]), reinterpret_cast<wdc3_section_header*>(&m_buffer[ofs]) + m_hdr->section_count);
	ofs += sizeof(wdc3_section_header) * m_hdr->section_count;

	m_fields = std::span<field_structure>(reinterpret_cast<field_structure*>(&m_buffer[ofs]), reinterpret_cast<field_structure*>(&m_buffer[ofs]) + m_hdr->total_field_count);
	ofs += sizeof(field_structure) * m_hdr->total_field_count;

	std::span<field_storage_info> field_infos = std::span<field_storage_info>(reinterpret_cast<field_storage_info*>(&m_buffer[ofs]), reinterpret_cast<field_storage_info*>(&m_buffer[ofs]) + m_hdr->field_storage_info_size / sizeof(field_storage_info));
	ofs += m_hdr->field_storage_info_size;

	m_pallet_data = std::span<char>(reinterpret_cast<char*>(&m_buffer[ofs]), reinterpret_cast<char*>(&m_buffer[ofs]) + m_hdr->pallet_data_size);
	ofs += m_hdr->pallet_data_size;
	m_common_data = std::span<char>(reinterpret_cast<char*>(&m_buffer[ofs]), reinterpret_cast<char*>(&m_buffer[ofs]) + m_hdr->common_data_size);
	ofs += m_hdr->common_data_size;

	uint32_t common_data_ofs = 0;
	uint32_t pallet_data_ofs = 0;
	m_field_infos.reserve(field_infos.size());
	for (auto& it : field_infos)
	{
		if (it.additional_data_size % 4)
			throw std::exception();

		std::span<uint32_t> common_data;
		std::span<uint32_t> pallet_data;
		if (it.storage_type == field_compression::field_compression_common_data)
		{
			common_data = std::span<uint32_t>(reinterpret_cast<uint32_t*>(&m_common_data[common_data_ofs]), it.additional_data_size / 4);
			common_data_ofs += it.additional_data_size;
		}
		else if (it.storage_type == field_compression::field_compression_bitpacked_indexed)
		{
			pallet_data = std::span<uint32_t>(reinterpret_cast<uint32_t*>(&m_pallet_data[pallet_data_ofs]), it.additional_data_size / 4);
			pallet_data_ofs += it.additional_data_size;
		}
		else if (it.storage_type == field_compression::field_compression_bitpacked_indexed_array)
		{
			pallet_data = std::span<uint32_t>(reinterpret_cast<uint32_t*>(&m_pallet_data[pallet_data_ofs]), it.additional_data_size / 4);
			pallet_data_ofs += it.additional_data_size;
		}
		m_field_infos.emplace_back(it, common_data, pallet_data);
	}

	m_sections.reserve(m_hdr->section_count);
	for (int i = 0; i < m_hdr->section_count; ++i)
		m_sections.emplace_back(m_hdr, m_buffer, &sectionHeaders[i]);

	// build map id => index
	uint32_t id_index = (m_hdr->flags & static_cast<uint32_t>(db2_flags::has_non_inline_ids)) ? 0 : m_hdr->id_index;

	for (uint32_t i = 0; i < m_hdr->record_count; ++i)
		m_id_map.emplace(GetRecord(i).GetField(id_index).AsUInt32(), i);
}

db2_record db2::GetRecord(uint32_t i)
{
	return db2_record(this, i);
}
db2_record db2::GetRecordById(uint32_t id)
{
	return db2_record(this, m_id_map[id]);
}
db2_record db2::operator[](uint32_t i)
{
	return GetRecord(i);
}
uint32_t db2::GetCount()
{
	return m_hdr->record_count;
}

db2_record::db2_record(db2* db2, uint32_t index)
	: m_index(index),
	m_db2(db2)
{
}

db2_field db2_record::GetField(uint32_t i)
{
	return db2_field(m_db2, m_index, i);
}
db2_field db2_record::operator[](uint32_t i)
{
	return GetField(i);
}
uint32_t db2_record::GetCount()
{
	return m_db2->m_hdr->total_field_count + ((m_db2->m_hdr->flags & static_cast<uint32_t>(db2_flags::has_non_inline_ids)) ? 1 : 0);
}


db2_section::db2_section(wdc3_db2_header* hdr, std::span<uint8_t> buffer, wdc3_section_header* section_hdr)
{
	m_hdr = section_hdr;
	m_recordSize = hdr->record_size;

	uint32_t ofs = section_hdr->file_offset;
	if ((hdr->flags & 0x1) == 0)
	{
		m_record_data = buffer.subspan(ofs, section_hdr->record_count * hdr->record_size);
		ofs += m_record_data.size();
		if (section_hdr->string_table_size > 0)
		{
			m_string_data = std::span<char>(reinterpret_cast<char*>(&buffer[ofs]), section_hdr->string_table_size);
			ofs += m_string_data.size();
		}
	}
	else
	{
		// Offset map records -- these records have null-terminated strings inlined, and
		// since they are variable-length, they are pointed to by an array of 6-byte offset+size pairs.
		//char variable_record_data[section_headers.offset_records_end - section_headers.file_offset];
	}

	if (section_hdr->id_list_size > 0)
		m_id_list = std::span(reinterpret_cast<uint32_t*>(&buffer[ofs]), section_hdr->id_list_size / 4);
	//if (section_hdr->copy_table_count > 0)
	//{
	//	struct copy_table_entry
	//	{
	//		uint32_t id_of_new_row;
	//		uint32_t id_of_copied_row;
	//	};
	//	copy_table_entry copy_table[section_hdr->copy_table_count];
	//}
	//struct offset_map_entry
	//{
	//	uint32_t offset;
	//	uint16_t size;
	//};
	//offset_map_entry offset_map[section_headers.offset_map_id_count];
	//if (section_headers.relationship_data_size > 0) {
	//	// In some tables, this relationship mapping replaced columns that were used only as a lookup, such as the SpellID in SpellX* tables.
	//	struct relationship_entry
	//	{
	//		// This is the id of the foreign key for the record, e.g. SpellID in SpellX* tables.
	//		uint32_t foreign_id;
	//		// This is the index of the record in record_data.  Note that this is *not* the record's own ID.
	//		uint32_t record_index;
	//	};
	//	struct relationship_mapping
	//	{
	//		uint32_t            num_entries;
	//		uint32_t            min_id;
	//		uint32_t            max_id;
	//		relationship_entry  entries[num_entries];
	//	};
	//	relationship_mapping relationship_map;
	//}
	//uint32_t offset_map_id_list[section_headers.offset_map_id_count];
}

// index is relative to this section
uint8_t* db2_section::GetRecordData(uint32_t i)
{
	return &m_record_data[m_recordSize * i];
}
// index is relative to this section
char* db2_section::GetStringData(uint32_t ofs)
{
	return &m_string_data[ofs];
}