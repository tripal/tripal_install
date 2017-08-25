; tripal_3_dev make file for d.o. usage

 ; REQUIRED ATTRIBUTES

 ; The Drush Make API version.
 api = 2

 ; The version of Drupal the profile is built for.
 core = 7.x

 ; It is also necessary to define project type to be core as well
 projects[drupal][type] = core

 ; However, if you're trying to define patches for core, need a -dev release,
 ; or want anything more fancy than an official release, you need to put all
 ; of that into a separate 'drupal-org-core.make' file. See below for details.
 ; In that case, leaving this as 'core = 7.x' is fine.

 ; OPTIONAL ATTRIBUTES

 ; Here you see the format of an array in a .make file. Text enclosed
 ; in brackets are array keys, and each set to the right of the last is
 ; a layer deeper in the array. Note that the root array element is
 ; not enclosed in brackets:
 ; root_element[first_key][next_key] = value

 ; The 'projects' attribute is where you define the modules/themes that
 ; are to be packaged with the profile. The first key is the short name
 ; of the project (as seen in the drupal.org/project/{projectshortname}
 ; URI).

 ; These projects are defined using the short form definition. You can
 ; use this form if you only want to declare the version of the project.
 ; The version is the value to the right of the core Drupal version in a full
 ; version string. For example, if you wanted to specify Views 7.x-3.1,
 ; you would use:

; +++++ Modules +++++

projects[ctools][version] = "1.12"
projects[ctools][subdir] = "contrib"

projects[date][version] = "2.10"
projects[date][subdir] = "contrib"

projects[devel][version] = "1.5"
projects[devel][subdir] = "contrib"

projects[profiler_builder][version] = "1.2"
projects[profiler_builder][subdir] = "contrib"

projects[ds][version] = "2.14"
projects[ds][subdir] = "contrib"

projects[features][version] = "2.10"
projects[features][subdir] = "contrib"

projects[uuid_features][version] = "1.0-rc1"
projects[uuid_features][subdir] = "contrib"

projects[field_group][version] = "1.5"
projects[field_group][subdir] = "contrib"

projects[field_group_table][version] = "1.6"
projects[field_group_table][subdir] = "contrib"

projects[link][version] = "1.4"
projects[link][subdir] = "contrib"

projects[node_export][version] = "3.1"
projects[node_export][subdir] = "contrib"

projects[entity][version] = "1.8"
projects[entity][subdir] = "contrib"

projects[libraries][version] = "2.3"
projects[libraries][subdir] = "contrib"

projects[node_export_webforms][version] = "1.0-rc4"
projects[node_export_webforms][subdir] = "contrib"

projects[redirect][version] = "1.0-rc3"
projects[redirect][subdir] = "contrib"

projects[token][version] = "1.7"
projects[token][subdir] = "contrib"

projects[tripal][version] = "3.0-rc1"
projects[tripal][subdir] = "contrib"

projects[uuid][version] = "1.0"
projects[uuid][subdir] = "contrib"

projects[jquery_update][version] = "2.7"
projects[jquery_update][subdir] = "contrib"

projects[views][version] = "3.16"
projects[views][subdir] = "contrib"

projects[webform][version] = "4.15"
projects[webform][subdir] = "contrib"


; +++++ Themes +++++

; bootstrap
projects[bootstrap][type] = "theme"
projects[bootstrap][version] = "3.14"
projects[bootstrap][subdir] = "contrib"

