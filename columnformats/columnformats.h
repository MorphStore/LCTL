/* 
 * File:   formats.h
 * Author: Juliana Hildebrandt
 *
 * Created on 16. März 2021, 09:36
 */

#ifndef LCTL_FORMATS_FORMATS_H
#define LCTL_FORMATS_FORMATS_H

#include "forbp/dynbp.h"
#include "forbp/dynforbp.h"
#include "forbp/statbp.h"
#include "forbp/statfordynbp.h"
#include "forbp/statforstatbp.h"

#include "delta/delta.h"

/*#include "../compare/staticbp_8_8_1.h"
#include "../compare/staticbp_8_8_2.h"
#include "../compare/staticbp_8_8_3.h"
#include "../compare/staticbp_8_8_4.h"
#include "../compare/staticbp_8_8_5.h"
#include "../compare/staticbp_8_8_6.h"
#include "../compare/staticbp_8_8_7.h"
#include "../compare/staticbp_8_8_8.h"
#include "../compare/staticbp_8_16_1.h"
#include "../compare/staticbp_8_16_2.h"
#include "../compare/staticbp_8_16_3.h"
#include "../compare/staticbp_8_16_4.h"
#include "../compare/staticbp_8_16_5.h"
#include "../compare/staticbp_8_16_6.h"
#include "../compare/staticbp_8_16_7.h"
#include "../compare/staticbp_8_16_8.h"
#include "../compare/staticbp_8_32_1.h"
#include "../compare/staticbp_8_32_2.h"
#include "../compare/staticbp_8_32_3.h"
#include "../compare/staticbp_8_32_4.h"
#include "../compare/staticbp_8_32_5.h"
#include "../compare/staticbp_8_32_6.h"
#include "../compare/staticbp_8_32_7.h"
#include "../compare/staticbp_8_32_8.h"
#include "../compare/staticbp_8_64_1.h"
#include "../compare/staticbp_8_64_2.h"
#include "../compare/staticbp_8_64_3.h"
#include "../compare/staticbp_8_64_4.h"
#include "../compare/staticbp_8_64_5.h"
#include "../compare/staticbp_8_64_6.h"
#include "../compare/staticbp_8_64_7.h"
#include "../compare/staticbp_8_64_8.h"
#include "../compare/staticbp_16_8_1.h"
#include "../compare/staticbp_16_8_2.h"
#include "../compare/staticbp_16_8_3.h"
#include "../compare/staticbp_16_8_4.h"
#include "../compare/staticbp_16_8_5.h"
#include "../compare/staticbp_16_8_6.h"
#include "../compare/staticbp_16_8_7.h"
#include "../compare/staticbp_16_8_8.h"
#include "../compare/staticbp_16_8_9.h"
#include "../compare/staticbp_16_8_10.h"
#include "../compare/staticbp_16_8_11.h"
#include "../compare/staticbp_16_8_12.h"
#include "../compare/staticbp_16_8_13.h"
#include "../compare/staticbp_16_8_14.h"
#include "../compare/staticbp_16_8_15.h"
#include "../compare/staticbp_16_8_16.h"
#include "../compare/staticbp_16_16_1.h"
#include "../compare/staticbp_16_16_2.h"
#include "../compare/staticbp_16_16_3.h"
#include "../compare/staticbp_16_16_4.h"
#include "../compare/staticbp_16_16_5.h"
#include "../compare/staticbp_16_16_6.h"
#include "../compare/staticbp_16_16_7.h"
#include "../compare/staticbp_16_16_8.h"
#include "../compare/staticbp_16_16_9.h"
#include "../compare/staticbp_16_16_10.h"
#include "../compare/staticbp_16_16_11.h"
#include "../compare/staticbp_16_16_12.h"
#include "../compare/staticbp_16_16_13.h"
#include "../compare/staticbp_16_16_14.h"
#include "../compare/staticbp_16_16_15.h"
#include "../compare/staticbp_16_16_16.h"
#include "../compare/staticbp_16_32_1.h"
#include "../compare/staticbp_16_32_2.h"
#include "../compare/staticbp_16_32_3.h"
#include "../compare/staticbp_16_32_4.h"
#include "../compare/staticbp_16_32_5.h"
#include "../compare/staticbp_16_32_6.h"
#include "../compare/staticbp_16_32_7.h"
#include "../compare/staticbp_16_32_8.h"
#include "../compare/staticbp_16_32_9.h"
#include "../compare/staticbp_16_32_10.h"
#include "../compare/staticbp_16_32_11.h"
#include "../compare/staticbp_16_32_12.h"
#include "../compare/staticbp_16_32_13.h"
#include "../compare/staticbp_16_32_14.h"
#include "../compare/staticbp_16_32_15.h"
#include "../compare/staticbp_16_32_16.h"
#include "../compare/staticbp_16_64_1.h"
#include "../compare/staticbp_16_64_2.h"
#include "../compare/staticbp_16_64_3.h"
#include "../compare/staticbp_16_64_4.h"
#include "../compare/staticbp_16_64_5.h"
#include "../compare/staticbp_16_64_6.h"
#include "../compare/staticbp_16_64_7.h"
#include "../compare/staticbp_16_64_8.h"
#include "../compare/staticbp_16_64_9.h"
#include "../compare/staticbp_16_64_10.h"
#include "../compare/staticbp_16_64_11.h"
#include "../compare/staticbp_16_64_12.h"
#include "../compare/staticbp_16_64_13.h"
#include "../compare/staticbp_16_64_14.h"
#include "../compare/staticbp_16_64_15.h"
#include "../compare/staticbp_16_64_16.h"
#include "../compare/staticbp_32_8_1.h"
#include "../compare/staticbp_32_8_2.h"
#include "../compare/staticbp_32_8_3.h"
#include "../compare/staticbp_32_8_4.h"
#include "../compare/staticbp_32_8_5.h"
#include "../compare/staticbp_32_8_6.h"
#include "../compare/staticbp_32_8_7.h"
#include "../compare/staticbp_32_8_8.h"
#include "../compare/staticbp_32_8_9.h"
#include "../compare/staticbp_32_8_10.h"
#include "../compare/staticbp_32_8_11.h"
#include "../compare/staticbp_32_8_12.h"
#include "../compare/staticbp_32_8_13.h"
#include "../compare/staticbp_32_8_14.h"
#include "../compare/staticbp_32_8_15.h"
#include "../compare/staticbp_32_8_16.h"
#include "../compare/staticbp_32_8_17.h"
#include "../compare/staticbp_32_8_18.h"
#include "../compare/staticbp_32_8_19.h"
#include "../compare/staticbp_32_8_20.h"
#include "../compare/staticbp_32_8_21.h"
#include "../compare/staticbp_32_8_22.h"
#include "../compare/staticbp_32_8_23.h"
#include "../compare/staticbp_32_8_24.h"
#include "../compare/staticbp_32_8_25.h"
#include "../compare/staticbp_32_8_26.h"
#include "../compare/staticbp_32_8_27.h"
#include "../compare/staticbp_32_8_28.h"
#include "../compare/staticbp_32_8_29.h"
#include "../compare/staticbp_32_8_30.h"
#include "../compare/staticbp_32_8_31.h"
#include "../compare/staticbp_32_8_32.h"
#include "../compare/staticbp_32_16_1.h"
#include "../compare/staticbp_32_16_2.h"
#include "../compare/staticbp_32_16_3.h"
#include "../compare/staticbp_32_16_4.h"
#include "../compare/staticbp_32_16_5.h"
#include "../compare/staticbp_32_16_6.h"
#include "../compare/staticbp_32_16_7.h"
#include "../compare/staticbp_32_16_8.h"
#include "../compare/staticbp_32_16_9.h"
#include "../compare/staticbp_32_16_10.h"
#include "../compare/staticbp_32_16_11.h"
#include "../compare/staticbp_32_16_12.h"
#include "../compare/staticbp_32_16_13.h"
#include "../compare/staticbp_32_16_14.h"
#include "../compare/staticbp_32_16_15.h"
#include "../compare/staticbp_32_16_16.h"
#include "../compare/staticbp_32_16_17.h"
#include "../compare/staticbp_32_16_18.h"
#include "../compare/staticbp_32_16_19.h"
#include "../compare/staticbp_32_16_20.h"
#include "../compare/staticbp_32_16_21.h"
#include "../compare/staticbp_32_16_22.h"
#include "../compare/staticbp_32_16_23.h"
#include "../compare/staticbp_32_16_24.h"
#include "../compare/staticbp_32_16_25.h"
#include "../compare/staticbp_32_16_26.h"
#include "../compare/staticbp_32_16_27.h"
#include "../compare/staticbp_32_16_28.h"
#include "../compare/staticbp_32_16_29.h"
#include "../compare/staticbp_32_16_30.h"
#include "../compare/staticbp_32_16_31.h"
#include "../compare/staticbp_32_16_32.h"
#include "../compare/staticbp_32_32_1.h"
#include "../compare/staticbp_32_32_2.h"
#include "../compare/staticbp_32_32_3.h"
#include "../compare/staticbp_32_32_4.h"
#include "../compare/staticbp_32_32_5.h"
#include "../compare/staticbp_32_32_6.h"
#include "../compare/staticbp_32_32_7.h"
#include "../compare/staticbp_32_32_8.h"
#include "../compare/staticbp_32_32_9.h"
#include "../compare/staticbp_32_32_10.h"
#include "../compare/staticbp_32_32_11.h"
#include "../compare/staticbp_32_32_12.h"
#include "../compare/staticbp_32_32_13.h"
#include "../compare/staticbp_32_32_14.h"
#include "../compare/staticbp_32_32_15.h"
#include "../compare/staticbp_32_32_16.h"
#include "../compare/staticbp_32_32_17.h"
#include "../compare/staticbp_32_32_18.h"
#include "../compare/staticbp_32_32_19.h"
#include "../compare/staticbp_32_32_20.h"
#include "../compare/staticbp_32_32_21.h"
#include "../compare/staticbp_32_32_22.h"
#include "../compare/staticbp_32_32_23.h"
#include "../compare/staticbp_32_32_24.h"
#include "../compare/staticbp_32_32_25.h"
#include "../compare/staticbp_32_32_26.h"
#include "../compare/staticbp_32_32_27.h"
#include "../compare/staticbp_32_32_28.h"
#include "../compare/staticbp_32_32_29.h"
#include "../compare/staticbp_32_32_30.h"
#include "../compare/staticbp_32_32_31.h"
#include "../compare/staticbp_32_32_32.h"
#include "../compare/staticbp_32_64_1.h"
#include "../compare/staticbp_32_64_2.h"
#include "../compare/staticbp_32_64_3.h"
#include "../compare/staticbp_32_64_4.h"
#include "../compare/staticbp_32_64_5.h"
#include "../compare/staticbp_32_64_6.h"
#include "../compare/staticbp_32_64_7.h"
#include "../compare/staticbp_32_64_8.h"
#include "../compare/staticbp_32_64_9.h"
#include "../compare/staticbp_32_64_10.h"
#include "../compare/staticbp_32_64_11.h"
#include "../compare/staticbp_32_64_12.h"
#include "../compare/staticbp_32_64_13.h"
#include "../compare/staticbp_32_64_14.h"
#include "../compare/staticbp_32_64_15.h"
#include "../compare/staticbp_32_64_16.h"
#include "../compare/staticbp_32_64_17.h"
#include "../compare/staticbp_32_64_18.h"
#include "../compare/staticbp_32_64_19.h"
#include "../compare/staticbp_32_64_20.h"
#include "../compare/staticbp_32_64_21.h"
#include "../compare/staticbp_32_64_22.h"
#include "../compare/staticbp_32_64_23.h"
#include "../compare/staticbp_32_64_24.h"
#include "../compare/staticbp_32_64_25.h"
#include "../compare/staticbp_32_64_26.h"
#include "../compare/staticbp_32_64_27.h"
#include "../compare/staticbp_32_64_28.h"
#include "../compare/staticbp_32_64_29.h"
#include "../compare/staticbp_32_64_30.h"
#include "../compare/staticbp_32_64_31.h"
#include "../compare/staticbp_32_64_32.h"
#include "../compare/staticbp_64_8_1.h"
#include "../compare/staticbp_64_8_2.h"
#include "../compare/staticbp_64_8_3.h"
#include "../compare/staticbp_64_8_4.h"
#include "../compare/staticbp_64_8_5.h"
#include "../compare/staticbp_64_8_6.h"
#include "../compare/staticbp_64_8_7.h"
#include "../compare/staticbp_64_8_8.h"
#include "../compare/staticbp_64_8_9.h"
#include "../compare/staticbp_64_8_10.h"
#include "../compare/staticbp_64_8_11.h"
#include "../compare/staticbp_64_8_12.h"
#include "../compare/staticbp_64_8_13.h"
#include "../compare/staticbp_64_8_14.h"
#include "../compare/staticbp_64_8_15.h"
#include "../compare/staticbp_64_8_16.h"
#include "../compare/staticbp_64_8_17.h"
#include "../compare/staticbp_64_8_18.h"
#include "../compare/staticbp_64_8_19.h"
#include "../compare/staticbp_64_8_20.h"
#include "../compare/staticbp_64_8_21.h"
#include "../compare/staticbp_64_8_22.h"
#include "../compare/staticbp_64_8_23.h"
#include "../compare/staticbp_64_8_24.h"
#include "../compare/staticbp_64_8_25.h"
#include "../compare/staticbp_64_8_26.h"
#include "../compare/staticbp_64_8_27.h"
#include "../compare/staticbp_64_8_28.h"
#include "../compare/staticbp_64_8_29.h"
#include "../compare/staticbp_64_8_30.h"
#include "../compare/staticbp_64_8_31.h"
#include "../compare/staticbp_64_8_32.h"
#include "../compare/staticbp_64_8_33.h"
#include "../compare/staticbp_64_8_34.h"
#include "../compare/staticbp_64_8_35.h"
#include "../compare/staticbp_64_8_36.h"
#include "../compare/staticbp_64_8_37.h"
#include "../compare/staticbp_64_8_38.h"
#include "../compare/staticbp_64_8_39.h"
#include "../compare/staticbp_64_8_40.h"
#include "../compare/staticbp_64_8_41.h"
#include "../compare/staticbp_64_8_42.h"
#include "../compare/staticbp_64_8_43.h"
#include "../compare/staticbp_64_8_44.h"
#include "../compare/staticbp_64_8_45.h"
#include "../compare/staticbp_64_8_46.h"
#include "../compare/staticbp_64_8_47.h"
#include "../compare/staticbp_64_8_48.h"
#include "../compare/staticbp_64_8_49.h"
#include "../compare/staticbp_64_8_50.h"
#include "../compare/staticbp_64_8_51.h"
#include "../compare/staticbp_64_8_52.h"
#include "../compare/staticbp_64_8_53.h"
#include "../compare/staticbp_64_8_54.h"
#include "../compare/staticbp_64_8_55.h"
#include "../compare/staticbp_64_8_56.h"
#include "../compare/staticbp_64_8_57.h"
#include "../compare/staticbp_64_8_58.h"
#include "../compare/staticbp_64_8_59.h"
#include "../compare/staticbp_64_8_60.h"
#include "../compare/staticbp_64_8_61.h"
#include "../compare/staticbp_64_8_62.h"
#include "../compare/staticbp_64_8_63.h"
#include "../compare/staticbp_64_8_64.h"
#include "../compare/staticbp_64_16_1.h"
#include "../compare/staticbp_64_16_2.h"
#include "../compare/staticbp_64_16_3.h"
#include "../compare/staticbp_64_16_4.h"
#include "../compare/staticbp_64_16_5.h"
#include "../compare/staticbp_64_16_6.h"
#include "../compare/staticbp_64_16_7.h"
#include "../compare/staticbp_64_16_8.h"
#include "../compare/staticbp_64_16_9.h"
#include "../compare/staticbp_64_16_10.h"
#include "../compare/staticbp_64_16_11.h"
#include "../compare/staticbp_64_16_12.h"
#include "../compare/staticbp_64_16_13.h"
#include "../compare/staticbp_64_16_14.h"
#include "../compare/staticbp_64_16_15.h"
#include "../compare/staticbp_64_16_16.h"
#include "../compare/staticbp_64_16_17.h"
#include "../compare/staticbp_64_16_18.h"
#include "../compare/staticbp_64_16_19.h"
#include "../compare/staticbp_64_16_20.h"
#include "../compare/staticbp_64_16_21.h"
#include "../compare/staticbp_64_16_22.h"
#include "../compare/staticbp_64_16_23.h"
#include "../compare/staticbp_64_16_24.h"
#include "../compare/staticbp_64_16_25.h"
#include "../compare/staticbp_64_16_26.h"
#include "../compare/staticbp_64_16_27.h"
#include "../compare/staticbp_64_16_28.h"
#include "../compare/staticbp_64_16_29.h"
#include "../compare/staticbp_64_16_30.h"
#include "../compare/staticbp_64_16_31.h"
#include "../compare/staticbp_64_16_32.h"
#include "../compare/staticbp_64_16_33.h"
#include "../compare/staticbp_64_16_34.h"
#include "../compare/staticbp_64_16_35.h"
#include "../compare/staticbp_64_16_36.h"
#include "../compare/staticbp_64_16_37.h"
#include "../compare/staticbp_64_16_38.h"
#include "../compare/staticbp_64_16_39.h"
#include "../compare/staticbp_64_16_40.h"
#include "../compare/staticbp_64_16_41.h"
#include "../compare/staticbp_64_16_42.h"
#include "../compare/staticbp_64_16_43.h"
#include "../compare/staticbp_64_16_44.h"
#include "../compare/staticbp_64_16_45.h"
#include "../compare/staticbp_64_16_46.h"
#include "../compare/staticbp_64_16_47.h"
#include "../compare/staticbp_64_16_48.h"
#include "../compare/staticbp_64_16_49.h"
#include "../compare/staticbp_64_16_50.h"
#include "../compare/staticbp_64_16_51.h"
#include "../compare/staticbp_64_16_52.h"
#include "../compare/staticbp_64_16_53.h"
#include "../compare/staticbp_64_16_54.h"
#include "../compare/staticbp_64_16_55.h"
#include "../compare/staticbp_64_16_56.h"
#include "../compare/staticbp_64_16_57.h"
#include "../compare/staticbp_64_16_58.h"
#include "../compare/staticbp_64_16_59.h"
#include "../compare/staticbp_64_16_60.h"
#include "../compare/staticbp_64_16_61.h"
#include "../compare/staticbp_64_16_62.h"
#include "../compare/staticbp_64_16_63.h"
#include "../compare/staticbp_64_16_64.h"
#include "../compare/staticbp_64_32_1.h"
#include "../compare/staticbp_64_32_2.h"
#include "../compare/staticbp_64_32_3.h"
#include "../compare/staticbp_64_32_4.h"
#include "../compare/staticbp_64_32_5.h"
#include "../compare/staticbp_64_32_6.h"
#include "../compare/staticbp_64_32_7.h"
#include "../compare/staticbp_64_32_8.h"
#include "../compare/staticbp_64_32_9.h"
#include "../compare/staticbp_64_32_10.h"
#include "../compare/staticbp_64_32_11.h"
#include "../compare/staticbp_64_32_12.h"
#include "../compare/staticbp_64_32_13.h"
#include "../compare/staticbp_64_32_14.h"
#include "../compare/staticbp_64_32_15.h"
#include "../compare/staticbp_64_32_16.h"
#include "../compare/staticbp_64_32_17.h"
#include "../compare/staticbp_64_32_18.h"
#include "../compare/staticbp_64_32_19.h"
#include "../compare/staticbp_64_32_20.h"
#include "../compare/staticbp_64_32_21.h"
#include "../compare/staticbp_64_32_22.h"
#include "../compare/staticbp_64_32_23.h"
#include "../compare/staticbp_64_32_24.h"
#include "../compare/staticbp_64_32_25.h"
#include "../compare/staticbp_64_32_26.h"
#include "../compare/staticbp_64_32_27.h"
#include "../compare/staticbp_64_32_28.h"
#include "../compare/staticbp_64_32_29.h"
#include "../compare/staticbp_64_32_30.h"
#include "../compare/staticbp_64_32_31.h"
#include "../compare/staticbp_64_32_32.h"
#include "../compare/staticbp_64_32_33.h"
#include "../compare/staticbp_64_32_34.h"
#include "../compare/staticbp_64_32_35.h"
#include "../compare/staticbp_64_32_36.h"
#include "../compare/staticbp_64_32_37.h"
#include "../compare/staticbp_64_32_38.h"
#include "../compare/staticbp_64_32_39.h"
#include "../compare/staticbp_64_32_40.h"
#include "../compare/staticbp_64_32_41.h"
#include "../compare/staticbp_64_32_42.h"
#include "../compare/staticbp_64_32_43.h"
#include "../compare/staticbp_64_32_44.h"
#include "../compare/staticbp_64_32_45.h"
#include "../compare/staticbp_64_32_46.h"
#include "../compare/staticbp_64_32_47.h"
#include "../compare/staticbp_64_32_48.h"
#include "../compare/staticbp_64_32_49.h"
#include "../compare/staticbp_64_32_50.h"
#include "../compare/staticbp_64_32_51.h"
#include "../compare/staticbp_64_32_52.h"
#include "../compare/staticbp_64_32_53.h"
#include "../compare/staticbp_64_32_54.h"
#include "../compare/staticbp_64_32_55.h"
#include "../compare/staticbp_64_32_56.h"
#include "../compare/staticbp_64_32_57.h"
#include "../compare/staticbp_64_32_58.h"
#include "../compare/staticbp_64_32_59.h"
#include "../compare/staticbp_64_32_60.h"
#include "../compare/staticbp_64_32_61.h"
#include "../compare/staticbp_64_32_62.h"
#include "../compare/staticbp_64_32_63.h"
#include "../compare/staticbp_64_32_64.h"
#include "../compare/staticbp_64_64_1.h"
#include "../compare/staticbp_64_64_2.h"
#include "../compare/staticbp_64_64_3.h"
#include "../compare/staticbp_64_64_4.h"
#include "../compare/staticbp_64_64_5.h"
#include "../compare/staticbp_64_64_6.h"
#include "../compare/staticbp_64_64_7.h"
#include "../compare/staticbp_64_64_8.h"
#include "../compare/staticbp_64_64_9.h"
#include "../compare/staticbp_64_64_10.h"
#include "../compare/staticbp_64_64_11.h"
#include "../compare/staticbp_64_64_12.h"
#include "../compare/staticbp_64_64_13.h"
#include "../compare/staticbp_64_64_14.h"
#include "../compare/staticbp_64_64_15.h"
#include "../compare/staticbp_64_64_16.h"
#include "../compare/staticbp_64_64_17.h"
#include "../compare/staticbp_64_64_18.h"
#include "../compare/staticbp_64_64_19.h"
#include "../compare/staticbp_64_64_20.h"
#include "../compare/staticbp_64_64_21.h"
#include "../compare/staticbp_64_64_22.h"
#include "../compare/staticbp_64_64_23.h"
#include "../compare/staticbp_64_64_24.h"
#include "../compare/staticbp_64_64_25.h"
#include "../compare/staticbp_64_64_26.h"
#include "../compare/staticbp_64_64_27.h"
#include "../compare/staticbp_64_64_28.h"
#include "../compare/staticbp_64_64_29.h"
#include "../compare/staticbp_64_64_30.h"
#include "../compare/staticbp_64_64_31.h"
#include "../compare/staticbp_64_64_32.h"
#include "../compare/staticbp_64_64_33.h"
#include "../compare/staticbp_64_64_34.h"
#include "../compare/staticbp_64_64_35.h"
#include "../compare/staticbp_64_64_36.h"
#include "../compare/staticbp_64_64_37.h"
#include "../compare/staticbp_64_64_38.h"
#include "../compare/staticbp_64_64_39.h"
#include "../compare/staticbp_64_64_40.h"
#include "../compare/staticbp_64_64_41.h"
#include "../compare/staticbp_64_64_42.h"
#include "../compare/staticbp_64_64_43.h"
#include "../compare/staticbp_64_64_44.h"
#include "../compare/staticbp_64_64_45.h"
#include "../compare/staticbp_64_64_46.h"
#include "../compare/staticbp_64_64_47.h"
#include "../compare/staticbp_64_64_48.h"
#include "../compare/staticbp_64_64_49.h"
#include "../compare/staticbp_64_64_50.h"
#include "../compare/staticbp_64_64_51.h"
#include "../compare/staticbp_64_64_52.h"
#include "../compare/staticbp_64_64_53.h"
#include "../compare/staticbp_64_64_54.h"
#include "../compare/staticbp_64_64_55.h"
#include "../compare/staticbp_64_64_56.h"
#include "../compare/staticbp_64_64_57.h"
#include "../compare/staticbp_64_64_58.h"
#include "../compare/staticbp_64_64_59.h"
#include "../compare/staticbp_64_64_60.h"
#include "../compare/staticbp_64_64_61.h"
#include "../compare/staticbp_64_64_62.h"
#include "../compare/staticbp_64_64_63.h"
#include "../compare/staticbp_64_64_64.h"*/

#endif /* LCTL_FORMATS_FORMATS_H */

