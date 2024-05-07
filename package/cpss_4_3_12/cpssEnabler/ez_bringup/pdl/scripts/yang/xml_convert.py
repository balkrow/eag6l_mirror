
import sys
import io
import os
import re
import xml.etree.ElementTree as ET
import codecs

is_debug = False

def fix_dict(convDict, duplicates=[]):
	# split dictionary keys into separate entries
	convDict2 = {}
	for kss in convDict.keys():
		v = convDict.get(kss, duplicates)
		if isinstance(v, dict):
			v,duplicates = fix_dict(v, duplicates)
		ks = kss.split(',')
		for k in ks:
			convDict2[k] = v
		if "add_parent" in convDict:
			v = convDict.get("add_parent")
			duplicates.append(v) if v not in duplicates else duplicates
	return convDict2,duplicates


def main():
	global is_debug
	prefix = ""
	convDict = {
		"MTS-Platform":
			{
				"add_tags":
					[("CPU-info-feature-is-supported", "leaf", "boolean", "from_tag", "CPU-feature-is-supported"),
					 ("Network-ports-feature-is-supported", "leaf", "boolean", "set", "true"),
					 ("Packet-processor-is-supported", "leaf", "boolean", "set", "true"),
					 ("Back-to-back-is-supported", "leaf", "boolean", "section_exists", ["Back-to-Back-Link-Information","Back-to-Back-links"], "true", "false"),
					 # ("Packet-Processors", "container", [("number-of-pps", "leaf", "integer", "section_exists", ["Back-to-Back-Link-Information","Back-to-Back-links"], "2", "1")])
					 ],
				"rename_tags":
					{
						"CPU-info-current": ("CPU-current-info", "in_section")
					},
				"LEDs":
					{
						"PortsLeds":
							{
								"PortLeds-list":
									{
										"LeftLed,RightLed,SingleLed":
											{
												"rename_tags":
													{
														"led-i2c": ("led-pp-i2c", "in_section"),
														"led-phy": ("led-pp-phy", "in_section"),
														"led-pp": ("led-pp-values", "in_section"),
														"led-gpio-color-list": ("led-pp-gpio-color-list", "in_section"),
														"led-color-list": ("led-pp-color-list", "in_section")
													},
												"led-color-list":
													{
														"rename_tags":
															{
																"led-i2c": ("led-pp-color-i2c", "in_section"),
																"led-phy": ("led-pp-color-phy", "in_section"),
																"led-pp": ("led-pp-color-pp", "in_section")
															}
													}
											},
									}
							},
						"SystemLed,RPSLed,PoELed,FanLed,PortLedStateLed":
							{
								"rename_tags":
									{
										"led-i2c": ("led-extdrv-i2c", "in_section"),
										"led-i2c-color-list": ("led-extdrv-i2c-color-list", "in_section"),
										"led-gpio-color-list": ("led-extdrv-gpio-color-list", "in_section"),
									},
								"led-i2c-color-list":
									{
										"remove_tags":
											{
												"led-i2c"
											}
									},
								"led-gpio-color-list":
									{
										"remove_tags":
											{
												"led-gpio"
											}
									}
							},
						"StackLeds":
							{
								"rename_tags":
									{
										"led-i2c": ("led-extdrv-i2c", "in_section"),
										"led-i2c-color-list": ("led-extdrv-i2c", "in_section"),
										"led-gpio-color-list": ("led-extdrv-gpio-color-list", "in_section"),
									},
								"led-i2c-color-list":
									{
										"remove_tags":
											{
												"led-i2c"
											}
									},
								"led-gpio-color-list":
									{
										"remove_tags":
											{
												"led-gpio"
											}
									},
								"stackLed-list":
									{
										"stack-led":
											{
												"rename_tags":
													{
														"led-i2c": ("led-extdrv-i2c", "in_section"),
														"led-i2c-color-list": ("led-extdrv-i2c", "in_section"),
														"led-gpio-color-list": ("led-extdrv-gpio-color-list", "in_section"),
													},
												"led-i2c-color-list":
													{
														"remove_tags":
															{
																"led-i2c"
															}
													},
												"led-gpio-color-list":
													{
														"remove_tags":
															{
																"led-gpio"
															}
													}
											}
									}
							},
						"LedstreamLeds":
							{
								"ledstream-interface-init-value":
									{
										"rename_tags":
											{
												"manipulation-list": (
												"ledstream-interface-manipulation-list", "in_section")

											}
									},
								"ledstream-interface-modes":
									{
										"ledstream-port-mode-list":
											{
												"rename_tags":
													{
														"manipulation-list": (
														"ledstream-port-manipulation-list", "in_section")

													}
											},
										"ledstream-group-type":
											{
												"rename_tags":
													{
														"interface-mode-list": (
														"ledstream-interface-mode-list", "in_section")

													}
											}
									}
							},
						"OOBLeds":
							{
								"OOBLeftLed,OOBRightLed":
									{
										"rename_tags":
											{
												"led-i2c": ("led-oob-i2c", "in_section"),
												"led-phy": ("led-oob-phy", "in_section"),
												"led-color-list": ("led-oob-color-list", "in_section"),
											},
										"led-phy":
											{
												"rename_tags":
													{
														"led-phy-off-value": ("led-oob-phy-off-value", "in_section"),
														"led-phy-init-list": ("led-oob-phy-init-list", "in_section"),
													},

											},
										"led-color-list":
											{
												"rename_tags":
													{
														"led-gpio": ("led-oob-color-gpio", "in_section"),
														"led-i2c": ("led-oob-color-i2c", "in_section"),
														"led-phy": ("led-oob-color-phy", "in_section"),
													},

											},
									}
							},
					},
				"Power":
					{
						"PS":
							{
								"rename_tags":
									{
										"activeInfo": ("PS-activeInfo", "in_section")
									},
								"activeInfo":
									{
										"rename_tags":
											{
												"i2c": ("power-connection-read-only-i2c", "in_section"),
												"GPIO-read-group-list": ("power-connection-read-only-gpio-group-list", "in_section")
											}
									}
							},
						"RPS":
							{
								"connectedInfo":
									{
										"rename_tags":
											{
												"i2c": ("power-connection-read-only-i2c", "in_section"),
												"GPIO-read-group-list": ("power-connection-read-only-gpio-group-list", "in_section")
											}
									},
								"status-list":
									{
										"status-value":
											{
												"rename_tags":
													{
														"i2c": ("power-connection-read-only-i2c", "in_section"),
														"GPIO-read-group-list": ("power-connection-read-only-gpio-group-list", "in_section")
													}
											}
									}
							}
					},
				"PoE":
					{
						"PSEports":
							{
								"PSEport-list":
									{
										"pse-group":
											{
												"rename_tags":
													{
														"activeInfo": ("pse-group-activeInfo", "in_section")
													},
												"activeInfo":
													{
														"rename_tags":
															{
																"i2c": ("power-connection-read-only-i2c", "in_section"),
																"GPIO-read-group-list": ("power-connection-read-only-gpio-group-list", "in_section")
															}
													}
											}
									}
							},
						"PDports":
							{
								"PDport-list":
									{
										"pd-group":
											{
												"ATinfo,AFinfo,sixstyWinfo":
													{
														"rename_tags":
															{
																"i2c": ("power-connection-read-only-i2c", "in_section"),
																"GPIO-read-group-list": ("power-connection-read-only-gpio-group-list", "in_section")
															}
													},
												"clearControl,ATcontrol,AFcontrol,sixstycontrol":
													{
														"rename_tags":
															{
																"i2c": ("power-connection-read-write-i2c", "in_section"),
																"gpio": ("power-connection-read-write-gpio", "in_section")
															}
													}
											}
									}
							}
					},
				"SERDES":
					{
						"Packet-Processor-list":
							{
								"lane-list":
									{
										"lane-attributes-info-list":
											{
												"rename_tags":
													{
														"tx-fine-tune": ("lane-attributes-info-tx-fine-tune", "in_section"),
														"rx-fine-tune": ("lane-attributes-info-rx-fine-tune", "in_section")
													}
											},
										"lane-attributes":
											{
												"rename_tags":
													{
														"tx-fine-tune": ("lane-attribute-tx-fine-tune", "in_section"),
														"rx-fine-tune": ("lane-attribute-rx-fine-tune", "in_section")
													}
											}
									}
							}
					},
				"FANs":
					{
						"fan-controller-list":
							{
								"rename_tags":
									{
										"i2c-read-write-address": ("fan-hw-i2c-read-write-address", "in_section")
									}
							},
						"sw-fan-controllers":
							{
								"threshold-list":
									{
										"rename_tags":
											{
												"sensor-list": ("sw-fan-controller-sensor-list", "in_section")
											}
									}
							}
					},
				"CPU-info,CPU-SDMA-port-mapping":
					{
						"add_parent": "CPU"
					},
				"CPU-info-current,CPU-SDMA-port-mapping":
					{
						"add_parent": "CPU"
					},
				"Board-description":
					{
						"add_parent": "Board-information"
					},

			}
	}

	if len(sys.argv) < 2:
		print("Incorrect args count. Program requires at least 1 parameter.")
		exit(1)
	if len(sys.argv) > 2:
		is_debug = sys.argv[2]

	new_filename = 'converted'

	if os.path.isdir(sys.argv[1]):
		files = [(os.path.join(sys.argv[1], file), os.path.join(sys.argv[1], new_filename + "/"  + file)) for file in os.listdir(sys.argv[1]) if os.path.isfile(os.path.join(sys.argv[1], file)) and file.endswith(".xml")]
	else:
		files = [(sys.argv[1], os.path.join(os.path.split(sys.argv[1])[0], new_filename + "/"  + os.path.split(sys.argv[1])[-1]))]


	convDict,duplicates = fix_dict(convDict)

	if os.path.isdir(sys.argv[1]):
		new_filename = "/".join([sys.argv[1], new_filename])
	else:
		new_filename = "/".join([os.path.split(sys.argv[1])[0], new_filename])

	if not os.path.exists(new_filename):
		os.makedirs(new_filename)

	BUFSIZE = 4096
	BOMLEN = len(codecs.BOM_UTF8)

	for old_filename,new_filename in files:
	# old_filename,new_filename = files[4]
	# if old_filename:
		print("Convert to new format file " +old_filename+" to "+new_filename)
		if is_debug:
			fdebug = open(new_filename + ".debug", mode="w")
		else:
			fdebug = None

		with open(old_filename, "r+b") as fp:
			chunk = fp.read(BUFSIZE)
			if chunk.startswith(codecs.BOM_UTF8):
				print("File "+old_filename+" is in non-supported UTF-8 BOM format - Converting to UTF-8")
				i = 0
				chunk = chunk[BOMLEN:]
				while chunk:
					fp.seek(i)
					fp.write(chunk)
					i += len(chunk)
					fp.seek(BOMLEN, os.SEEK_CUR)
					chunk = fp.read(BUFSIZE)
				fp.seek(-BOMLEN, os.SEEK_CUR)
				fp.truncate()

		with open(old_filename, mode="r") as fout, open(new_filename, mode="w") as fin:
			parse_file(fout, fin, fdebug, convDict)
			if duplicates:
				fin.close()
				fin = open(new_filename, mode="r+")
				lines = fin.readlines()
				prev_line = ''
				removed = []
				for i in range(len(lines)):
					curr_line = lines[i]
					for d in duplicates:
						if re.match("<mtsplt:"+d+">\n", curr_line) and re.match("</mtsplt:"+d+">\n", prev_line):
							removed.append(i)
							removed.append(i-1)
					prev_line = curr_line
				fin.close()
				fin = open(new_filename, mode="w")
				for i in range(len(lines)):
					if i not in removed:
						fin.write(lines[i])
		if is_debug:
			fdebug.close()

def add_tag(dict, fin, fout_name, prefix, ident):
	global is_debug
	whitespace = "".join([" " for i in range(ident)])
	for a in dict:
		if is_debug:
			print("add "+str(a)+" to file")
		value = ''
		if a[1] == "leaf":
			if a[3] == "set":
				value = a[4]
			elif a[3] == "from_tag":
				xml = ET.parse(fout_name)
				iter = xml.iter()
				try:
					while True:
						v = next(iter)
						if re.match("\{.*?\}" + a[4], v.tag):
							value = v.text
							break
				except:
					pass
			elif a[3] == "section_exists":
				value = a[6]
				for x in a[4]:
					xml = ET.parse(fout_name)
					iter = xml.iter()
					try:
						while True:
							v = next(iter)
							if re.match("\{.*?\}" + x, v.tag):
								value = a[5]
								break
					except:
						pass
					if value == a[5]:
						break;


			if value != '':
				fin.write(whitespace + "<" + prefix + a[0] + " "  +prefix +"type=\"" + a[2] +  "\">" + value + "</" + prefix + a[0] + ">\n")
			else:
				fin.write(whitespace + "<"+prefix+a[0]+"/>\n")

		elif a[1] == "container":
					fin.write(whitespace + "<"+prefix+a[0]+">\n")
					add_tag(a[2], fin, fout_name, prefix, ident + 4)
					fin.write(whitespace + "</"+prefix+a[0]+">\n")



def parse_file(fout, fin, fdebug, dict, end_tag_reg="xxx", prefix = "mtsplt:", ident=0, f_ptr = 0, line = 'xxx'):
	global is_debug
	whitespace = "".join([" " for i in range(ident)])
	open_parent = None
	while line:
		write_line = True
		after_recursion = False
		line = fout.readline()
		if is_debug:
			fdebug.write("\nread line >>"+line+"<<<")

		line_after_rename = line

		renames = dict.get("rename_tags")
		if renames:
			for r in renames.keys():
				if is_debug:
					fdebug.write("\nmatch " + r"<\s*"+prefix+r+"\s*>" + " to "+renames.get(r))
				if re.match("<\s*" + prefix + r+"\s*>", line_after_rename.strip()):
					if is_debug:
						fdebug.write("\nrename "+r+" to "+renames.get(r))
					line_after_rename = whitespace + line_after_rename.lstrip().replace(r, renames.get(r)[0])
					if is_debug:
						fdebug.write("\nline_after_rename >>"+line_after_rename+"<<<")
					break

		for k in dict.keys():
			if is_debug:
				fdebug.write("\nmatch " + r"<"+prefix+k+"\s*>" + " to "+line.strip())
			if re.match('<'+prefix+k+'[\s*>]', line.strip()):

				if is_debug:
					fdebug.write("\nfound "+prefix+k)

				open_parent = dict.get(k).get("add_parent")
				if open_parent:
					if is_debug:
						fdebug.write("\nadd open parent tag "+open_parent+" to file")
					fin.write(whitespace + "<"+prefix+open_parent+">\n")

				dont = False
				dels = dict.get(k).get("remove_tags")
				if dels:
					for d in dels:
						if re.match("<\s*/?\s*?" + prefix + d + "\s*?>", line.strip()):
							if is_debug:
								fdebug.write("\nremove tag "+prefix+d)
							dont = True
				if not dont:
					if is_debug:
						fdebug.write("\nwrite line >>>"+line_after_rename+"<<<")
					fin.write(line_after_rename)
				write_line = False


				adds = dict.get(k).get("add_tags")
				if adds:
					add_tag(adds, fin, fout.name, prefix, ident + 4)

				# if line.find(prefix+k) != -1:
				if is_debug:
					fdebug.write("\ngo into for tag "+prefix+k)
				line = parse_file(fout, fin, fdebug, dict.get(k), '^</'+prefix+k+'[\s*>]', prefix, ident+4, fout.tell())
				line_after_rename = line
				if is_debug:
					fdebug.write("\nupdate line_after_rename to >>>"+line_after_rename+"><<<")
				after_recursion = True

		renames = dict.get("rename_tags")
		if renames:
			for r in renames.keys():
				if re.match("<\s*/" + prefix + r+"\s*>", line.strip()):
					if is_debug:
						fdebug.write("\nrename "+r+" to "+renames.get(r))
					line_after_rename = whitespace + line.lstrip().replace(r, renames.get(r)[0])
					if is_debug:
						fdebug.write("\nupdate line_after_rename to >>>"+line_after_rename+"><<<")
					break

		if not after_recursion and re.match(end_tag_reg, line.strip()):
			if is_debug:
				fdebug.write("\nstop w/o recursion for tag "+end_tag_reg+" line >>>"+line.strip()+"<<<")
			return line

		if write_line or re.match("<\s*/\s*?(.+)?\s*?>", line.strip()):
			dont = False
			dels = dict.get("remove_tags")
			if dels:
				for d in dels:
					if re.match("<\s*/?\s*?" + prefix + d + "\s*?>", line.strip()):
						if is_debug:
							fdebug.write("\nremove tag "+prefix+d)
						dont = True
			if not dont:
				if after_recursion:
					renames = dict.get("rename_tags")
					if renames:
						for r in renames.keys():
							if re.match("<\s*/" + prefix + r + "\s*>", line_after_rename.strip()):
								if is_debug:
									fdebug.write("\nrename "+r+" to "+renames.get(r))
								line_after_rename = whitespace + line_after_rename.lstrip().replace(r, renames.get(r)[0])
								if is_debug:
									fdebug.write("\nupdate line_after_rename to >>>"+line_after_rename+"><<<")
								break

				if is_debug:
					fdebug.write("\nwrite line >>>"+line_after_rename+"><<<")
				fin.write(line_after_rename)
				if open_parent:
					whitespace = "".join([" " for i in range(ident)])
					if is_debug:
						fdebug.write("\nwrite line >>>"+whitespace+"</"+prefix+open_parent+"><<<")
					fin.write(whitespace + "</"+prefix+open_parent+">\n")
					open_parent = None

		if re.match(end_tag_reg, line.strip()):
			if is_debug:
				fdebug.write("\nstop recursion for tag "+end_tag_reg+" line >>>"+line.strip()+"<<<")
			return line


if __name__ == '__main__':
	main()
